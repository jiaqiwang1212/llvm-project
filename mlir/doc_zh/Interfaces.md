# 接口

MLIR 是一个通用且可扩展的框架，用不同的方言表示各自的属性、操作、类型等。MLIR 方言可以表达具有各种语义和不同抽象层次的操作。其缺点是 MLIR 的变换和分析需要能够处理每种操作的语义，否则就过于保守。如果不加注意，这可能导致代码中为每种受支持的操作类型设置特殊情况。为了解决这个问题，MLIR 提供了 `interfaces`（接口）的概念。

[TOC]

## 动机

接口提供了一种与 IR 交互的通用方式。其目标是能够用这些接口来表达变换/分析，而无需编码关于所涉及的具体操作或方言的特定知识。这使得编译器更易于扩展，允许以与变换/分析实现解耦的方式添加新的方言和操作。

### 方言接口

方言接口通常对变换 pass 或分析很有用，这些 pass 或分析希望在可能在不同方言中定义的一组属性/操作/类型上通用地操作。这些接口通常覆盖整个方言，且仅用于少数几个分析或变换。在这些情况下，直接在每个操作上注册接口过于复杂和繁琐。接口不是操作的核心，而只是针对特定变换的。这类接口的使用示例是内联（inlining）。内联通常查询方言中操作的高级信息，如成本建模和合法性，这通常不特定于某个操作。

方言接口可以通过继承 [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) 基类 `DialectInterfaceBase::Base<>` 来定义。此类提供了必要的实用程序，用于向方言注册接口，以便以后可以引用它。一旦定义了接口，方言可以使用方言特定信息来覆盖它。方言定义的接口通过 `addInterfaces<>` 注册，这与属性、操作、类型等的机制类似。

```c++
/// 定义一个基础内联接口类，允许方言选择加入内联器。
class DialectInlinerInterface :
    public DialectInterface::Base<DialectInlinerInterface> {
public:
  /// 如果给定的区域 'src' 可以内联到附加到当前方言注册操作的区域
  /// 'dest' 中，则返回 true。'valueMapping' 包含来自 'src' 区域内的
  /// 任何重新映射的值。例如，这可用于检查哪些值将替换 'src' 区域的
  /// 入口参数。
  virtual bool isLegalToInline(Region *dest, Region *src,
                               IRMapping &valueMapping) const {
    return false;
  }
};

/// 重写内联接口以添加对 AffineDialect 的支持，以启用内联 affine 操作。
struct AffineInlinerInterface : public DialectInlinerInterface {
  /// Affine 结构具有特定的内联约束。
  bool isLegalToInline(Region *dest, Region *src,
                       IRMapping &valueMapping) const final {
    ...
  }
};

/// 向方言注册接口。
AffineDialect::AffineDialect(MLIRContext *context) ... {
  addInterfaces<AffineInlinerInterface>();
}
```

一旦注册，这些接口可以从方言中被分析或变换查询，而无需确定具体的方言子类：

```c++
Dialect *dialect = ...;
if (DialectInlinerInterface *interface = dyn_cast<DialectInlinerInterface>(dialect)) {
  // 方言提供了此接口的实现。
  ...
}
```

#### 利用 ODS 框架

注意：在阅读本节之前，读者应该对
[`操作定义规范`](DefiningDialects/Operations.md) 文档中描述的概念有所了解。

MLIR 还支持直接在 **TableGen** 中定义方言接口。这减少了样板代码，允许作者以声明式方式指定高级接口结构。

例如，上述接口可以使用 ODS 定义如下：

```tablegen
def DialectInlinerInterface : DialectInterface<"DialectInlinerInterface"> {
  let description = [{
     定义一个基础内联接口类，允许方言选择加入内联器。
  }];

  let methods = [
    InterfaceMethod<[{
        如果给定的区域 'src' 可以内联到附加到当前方言注册操作的区域
        'dest' 中，则返回 true。'valueMapping' 包含来自 'src' 区域内的
        任何重新映射的值。例如，这可用于检查哪些值将替换 'src' 区域的
        入口参数。
      }],
      "bool", "isLegalToInline",
      (ins "Region *":$dest, "Region *":$src, "IRMapping &":$valueMapping),
      [{
        return false;
      }]
      >
  ];
}
```

`DialectInterfaces` 类使用以下组件：

*   C++ 类名（通过模板参数提供）
    -   C++ 接口类的名称。
*   描述（`description`）
    -   接口的字符串描述、其不变量、示例用法等。
*   C++ 命名空间（`cppNamespace`）
    -   应在其中生成接口类的 C++ 命名空间。
*   方法（`methods`）
    -   由 IR 对象定义的接口钩子方法列表。
    -   这些方法的结构定义在[此处](#dialect-interface-methods)。

可以通过以下命令生成头文件：

```bash
mlir-tblgen --gen-dialect-interface-decls DialectInterface.td
```

要在 CMake 中使用 ODS 框架生成方言接口声明，您可以这样写：

```cmake
set(LLVM_TARGET_DEFINITIONS DialectInlinerInterface.td)
mlir_tablegen(DialectInlinerInterface.h.inc -gen-dialect-interface-decls)
```

可以在 `mlir/include/mlir/Transforms` 下的 DialectInlinerInterface 实现及相关 `CMakeLists.txt` 中找到一个示例。

##### 方言接口方法

方言接口可以使用三种类型的方法：`InterfaceMethod`、`InterfaceMethodDeclaration` 和 `PureVirtualInterfaceMethod`。它们都由相同的核心组件组成，区别在于 `InterfaceMethod` 还支持默认方法体。

接口方法由以下组件组成：

*   描述：此方法的字符串描述、其不变量、示例用法等。
*   ReturnType：对应于方法 C++ 返回类型的字符串。
*   MethodName：对应于方法 C++ 名称的字符串。
*   Arguments（可选）：对应于 C++ 类型和变量名的字符串 dag。
*   MethodBody（可选，仅在 `InterfaceMethod` 中）：接口方法的可选显式实现。

`InterfaceMethodDeclaration` 只会声明类方法。另一方面，`PureVirtualInterfaceMethod` 将方法标记为纯虚函数，同时也使方言类的构造函数受保护。

#### DialectInterfaceCollection

通过 `DialectInterfaceCollection` 提供了一个额外的实用程序。此类允许收集在 `MLIRContext` 实例中注册了给定接口的所有方言。这对于隐藏和优化已注册方言接口的查找很有用。

```c++
class InlinerInterface : public
    DialectInterfaceCollection<DialectInlinerInterface> {
  /// 此类的钩子与 DialectInlinerInterface 的钩子对应，
  /// 使用默认实现调用给定方言接口上的钩子。
  virtual bool isLegalToInline(Region *dest, Region *src,
                               IRMapping &valueMapping) const {
    auto *handler = getInterfaceFor(dest->getContainingOp());
    return handler ? handler->isLegalToInline(dest, src, valueMapping) : false;
  }
};

MLIRContext *ctx = ...;
InlinerInterface interface(ctx);
if(!interface.isLegalToInline(...))
   ...
```

### 属性/操作/类型接口

顾名思义，属性/操作/类型接口是在特定属性/操作/类型级别注册的接口。这些接口通过提供必须实现的虚拟接口来提供对派生对象的访问。例如，许多分析和变换希望推理操作的副作用以提高性能和正确性。操作的副作用通常与特定操作的语义相关联，例如 `affine.load` 操作具有 `read` 效果（如名称所示）。

这些接口通过重写特定 IR 实体的 [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) 类来定义；分别是 `AttrInterface`、`OpInterface` 或 `TypeInterface`。这些类接受一个 `Traits` 类作为模板参数，该类定义了 `Concept` 类和 `Model` 类。这些类提供了基于概念的多态性的实现，其中 `Concept` 定义了一组虚拟方法，这些方法被模板化在具体实体类型上的 `Model` 重写。重要的是，这些类应该是纯净的，不应包含非静态数据成员或其他可变数据。要将接口附加到对象，基础接口类提供了一个 [`Trait`](Traits) 类，可以附加到该对象的 trait 列表中。

```c++
struct ExampleOpInterfaceTraits {
  /// 定义一个基础概念类，指定要实现的虚拟接口。
  struct Concept {
    virtual ~Concept();

    /// 这是操作的非静态钩子示例。
    virtual unsigned exampleInterfaceHook(Operation *op) const = 0;

    /// 这是操作的静态钩子示例。静态钩子不需要操作的具体实例。
    /// 实现是一个虚拟钩子，与非静态情况相同，因为钩子本身的
    /// 实现仍然需要间接寻址。
    virtual unsigned exampleStaticInterfaceHook() const = 0;
  };

  /// 定义一个在给定操作类型上特化概念的模型类。
  template <typename ConcreteOp>
  struct Model : public Concept {
    /// 重写方法以在具体操作上分派。
    unsigned exampleInterfaceHook(Operation *op) const final {
      return llvm::cast<ConcreteOp>(op).exampleInterfaceHook();
    }

    /// 重写静态方法以分派到具体操作类型。
    unsigned exampleStaticInterfaceHook() const final {
      return ConcreteOp::exampleStaticInterfaceHook();
    }
  };
};

/// 定义分析和变换将与之交互的主接口类。
class ExampleOpInterface : public OpInterface<ExampleOpInterface,
                                              ExampleOpInterfaceTraits> {
public:
  /// 继承基类构造函数以支持 LLVM 风格的转型。
  using OpInterface<ExampleOpInterface, ExampleOpInterfaceTraits>::OpInterface;

  /// 接口分派到 'getImpl()'，这是由基础 `OpInterface` 类提供的方法，
  /// 返回概念的实例。
  unsigned exampleInterfaceHook() const {
    return getImpl()->exampleInterfaceHook(getOperation());
  }
  unsigned exampleStaticInterfaceHook() const {
    return getImpl()->exampleStaticInterfaceHook(getOperation()->getName());
  }
};

```

一旦定义了接口，就通过如前所述添加提供的 trait `ExampleOpInterface::Trait` 将其注册到操作中。使用此接口就像使用任何其他派生操作类型一样，即转型：

```c++
/// 在定义操作时，接口通过 'OpInterface<>' 基类提供的嵌套 'Trait' 类注册。
class MyOp : public Op<MyOp, ExampleOpInterface::Trait> {
public:
  /// 在派生操作上的接口方法定义。
  unsigned exampleInterfaceHook() { return ...; }
  static unsigned exampleStaticInterfaceHook() { return ...; }
};

/// 稍后，我们可以查询特定操作（如 'MyOp'）是否实现了给定接口。
Operation *op = ...;
if (ExampleOpInterface example = dyn_cast<ExampleOpInterface>(op))
  llvm::errs() << "hook returned = " << example.exampleInterfaceHook() << "\n";
```

#### 属性、操作和类型接口的外部模型

可能希望在不修改 IR 对象定义的情况下为其提供接口实现。值得注意的是，这允许在定义这些对象的方言之外为属性、操作和类型实现接口，例如为内置类型提供接口。

这通过从 `Concept` 派生出两个更多类来扩展基于概念的多态性模型来实现，如下所示。

```c++
struct ExampleTypeInterfaceTraits {
  struct Concept {
    virtual unsigned exampleInterfaceHook(Type type) const = 0;
    virtual unsigned exampleStaticInterfaceHook() const = 0;
  };

  template <typename ConcreteType>
  struct Model : public Concept { /*...*/ };

  /// 与 `Model` 不同，`FallbackModel` 通过钩子传递类型对象，
  /// 使其在方法体中可访问，即使方法不是在类本身中定义的，
  /// 因此没有 `this` 访问。ODS 自动为所有接口生成此类。
  template <typename ConcreteType>
  struct FallbackModel : public Concept {
    unsigned exampleInterfaceHook(Type type) const override {
      getImpl()->exampleInterfaceHook(type);
    }
    unsigned exampleStaticInterfaceHook() const override {
      ConcreteType::exampleStaticInterfaceHook();
    }
  };

  /// `ExternalModel` 通过显式地将实现接口的模型类与实现接口的类型类
  /// 分开，为接口方法的默认实现提供了一个位置。默认实现可以泛型地
  /// 使用 `cast<ConcreteType>` 来定义。如果 `ConcreteType` 不提供
  /// 默认实现所需的 API，自定义实现可以直接使用 `FallbackModel` 来
  /// 覆盖默认实现。由于位于类模板中，它永远不会被实例化，也不会导致
  /// 编译错误。ODS 自动生成此类并将默认方法实现放入其中。
  template <typename ConcreteModel, typename ConcreteType>
  struct ExternalModel : public FallbackModel<ConcreteModel> {
    unsigned exampleInterfaceHook(Type type) const override {
      // 可以在此提供默认实现。
      return cast<ConcreteType>(type).callSomeTypeSpecificMethod();
    }
  };
};
```

可以通过派生 `FallbackModel` 或 `ExternalModel` 并在给定上下文中向相关类注册模型类，为属性、操作和类型接口提供外部模型。其他上下文不会看到该接口，除非已注册。

```c++
/// 具体类的外部接口实现。这不需要修改类型类本身的定义。
struct ExternalModelExample
    : public ExampleTypeInterface::ExternalModel<ExternalModelExample,
                                                 IntegerType> {
  static unsigned exampleStaticInterfaceHook() {
    // 在此提供实现。
    return IntegerType::someStaticMethod();
  }

  // 不需要定义在 `ExternalModel` 中有默认实现的 `exampleInterfaceHook`。
  // 但如果需要，可以覆盖它。
}

int main() {
  MLIRContext context;
  /* ... */;

  // 在使用之前，将接口模型附加到给定上下文中的类型。
  // 预计此时包含该类型的方言已被加载。
  IntegerType::attachInterface<ExternalModelExample>(context);
}
```

注意：强烈建议只有在您"拥有"正在外部应用的接口时才使用此机制。这可以防止出现这样的情况：包含对象的方言的所有者和接口的所有者都不知道接口实现，这可能导致重复或不一致的实现。

忘记注册外部模型可能导致难以追踪的错误。可以使用 `declarePromisedInterface` 函数来声明最终必须提供操作的外部模型实现。

```
  void MyDialect::initialize() {
    declarePromisedInterface<SomeInterface, SomeOp>();
     ...
  }
```

现在，在没有预先注册外部模型的情况下尝试使用接口（例如在转型中）将导致运行时错误，看起来类似于：

```
LLVM ERROR: checking for an interface (`SomeInterface`) that was promised by dialect 'mydialect' but never implemented. This is generally an indication that the dialect extension implementing the interface was never registered.
```

如果您遇到由 MLIR 提供的方言和接口的此错误，可以查找名为
`register<Dialect><Interface>ExternalModels(DialectRegistry &registry);` 的方法；尝试使用 `git grep 'register.*SomeInterface.*Model' mlir` 来查找它。

#### 针对 OpInterface 的方言回退

某些方言具有开放的生态系统，不会注册所有可能的操作。在这种情况下，仍然可以为这些操作提供实现 `OpInterface` 的支持。当操作未注册或未提供接口实现时，查询将回退到方言本身。

在使用 ODS 时（见下文），会自动生成名为 `FallbackModel` 的第二个模型用于此类情况。可以为特定方言实现此模型：

```c++
// 这是 `ExampleOpInterface` 的方言回退实现。
struct FallbackExampleOpInterface
    : public ExampleOpInterface::FallbackModel<
          FallbackExampleOpInterface> {
  static bool classof(Operation *op) { return true; }

  unsigned exampleInterfaceHook(Operation *op) const;
  unsigned exampleStaticInterfaceHook() const;
};
```

方言可以实例化此实现，并通过重写 `getRegisteredInterfaceForOp` 方法在特定操作上返回它：

```c++
void *TestDialect::getRegisteredInterfaceForOp(TypeID typeID,
                                               StringAttr opName) {
  if (typeID == TypeID::get<ExampleOpInterface>()) {
    if (isSupported(opName))
      return fallbackExampleOpInterface;
    return nullptr;
  }
  return nullptr;
}
```

#### 利用 ODS 框架

如上所述，[接口](#attributeoperationtype-interfaces)允许属性、操作和类型公开方法调用，而无需调用者知道具体的派生类型。此基础设施的缺点是需要一些样板代码将所有部分连接在一起。MLIR 提供了一种机制，可以在 ODS 中声明式定义接口，并自动生成 C++ 定义。

例如，使用 ODS 框架可以将上面的示例接口定义为：

```tablegen
def ExampleOpInterface : OpInterface<"ExampleOpInterface"> {
  let description = [{
    这是一个示例接口定义。
  }];

  let methods = [
    InterfaceMethod<
      "这是操作的非静态钩子示例。",
      "unsigned", "exampleInterfaceHook"
    >,
    StaticInterfaceMethod<
      "这是操作的静态钩子示例。",
      "unsigned", "exampleStaticInterfaceHook"
    >,
  ];
}
```

提供 `AttrInterface`、`OpInterface` 或 `TypeInterface` 类的定义将自动生成接口的 C++ 类。接口由以下组件组成：

*   C++ 类名（通过模板参数提供）
    -   C++ 接口类的名称。
*   接口基类
    -   接口类应继承的一组接口。有关更多详细信息，请参见下面的[接口继承](#interface-inheritance)。
*   描述（`description`）
    -   接口的字符串描述、其不变量、示例用法等。
*   C++ 命名空间（`cppNamespace`）
    -   应在其中生成接口类的 C++ 命名空间。
*   方法（`methods`）
    -   由 IR 对象定义的接口钩子方法列表。
    -   这些方法的结构定义如下。
*   额外类声明（可选：`extraClassDeclaration`）
    -   在接口类声明中生成的额外 C++ 代码。这允许在面向用户的接口类上定义方法等，而无需钩入 IR 实体。这些声明_不_在接口方法的默认实现中隐式可见，但静态声明可以通过完整的名称限定来访问。
*   额外共享类声明（可选：`extraSharedClassDeclaration`）
    -   注入到接口类和 trait 类声明中的额外 C++ 代码。这允许定义在接口和 trait 类上都公开的方法等，例如在接口和实现接口的派生实体（如属性、操作等）上注入实用程序。
    -   在非静态方法中，可以使用 `$_attr`/`$_op`/`$_type`（取决于接口类型）来引用 IR 实体的实例。在接口声明中，实例的类型是接口类。在 trait 声明中，实例的类型是具体实体类（例如 `IntegerAttr`、`FuncOp` 等）。
*   额外 Trait 类声明（可选：`extraTraitClassDeclaration`）
    -   注入到接口 trait 声明中的额外 C++ 代码。
    -   允许与额外共享类声明相同的替换。

`OpInterface` 类还可以包含以下内容：

*   验证器（`verify`）
    -   包含附加验证的 C++ 代码块，应用于附加了该接口的操作。
    -   此代码块的结构与 [`Trait::verifyTrait`](Traits) 方法的结构 1-1 对应。

##### 接口方法

属性/操作/类型接口可以使用两种类型的方法：`InterfaceMethod` 和 `StaticInterfaceMethod`。它们都由相同的核心组件组成，区别在于 `StaticInterfaceMethod` 模拟派生 IR 对象上的静态方法。

接口方法由以下组件组成：

*   描述
    -   此方法的字符串描述、其不变量、示例用法等。
*   ReturnType
    -   对应于方法 C++ 返回类型的字符串。
*   MethodName
    -   对应于方法 C++ 名称的字符串。
*   Arguments（可选）
    -   对应于 C++ 类型和变量名的字符串 dag。
*   MethodBody（可选）
    -   接口方法的可选显式实现。
    -   此实现放置在 `Model` traits 类上定义的方法中，不由附加到 IR 实体的 `Trait` 类定义。更具体地说，此主体仅对接口类可见，不影响派生 IR 实体。
    -   `ConcreteAttr`/`ConcreteOp`/`ConcreteType` 是一个隐式定义的 `typename`，可用于引用当前正在操作的派生 IR 实体的类型。
    -   在非静态方法中，`$_op` 和 `$_self` 可用于引用派生 IR 实体的实例。
*   DefaultImplementation（可选）
    -   接口方法的可选显式默认实现。
    -   此实现放置在附加到 IR 实体的 `Trait` 类中，不直接影响任何接口类。因此，此方法与任何其他 [`Trait`](Traits) 方法具有相同的特征。
    -   `ConcreteAttr`/`ConcreteOp`/`ConcreteType` 是一个隐式定义的 `typename`，可用于引用当前正在操作的派生 IR 实体的类型。
    -   这可以使用限定名称引用接口类的静态字段，例如 `TestOpInterface::staticMethod()`。

如果操作使用 `DeclareOpInterfaceMethods` 指定接口（见下面的示例），ODS 还允许为操作的 `InterfaceMethod` 生成声明。

示例：

```tablegen
def MyInterface : OpInterface<"MyInterface"> {
  let description = [{
    这是接口的描述。它提供了有关接口语义以及编译器如何使用它的具体信息。
  }];

  let methods = [
    InterfaceMethod<[{
      此方法表示一个没有输入且返回类型为 void 的简单非静态接口方法。
      实现此接口的所有操作都必须实现此方法。此方法大致对应于实现此接口的
      操作上的以下内容：

      ```c++
      class ConcreteOp ... {
      public:
        void nonStaticMethod();
      };
      ```
    }], "void", "nonStaticMethod"
    >,

    InterfaceMethod<[{
      此方法表示一个非静态接口方法，具有非 void 返回值以及名为 `i` 的
      `unsigned` 输入。实现此接口的所有操作都必须实现此方法。此方法大致
      对应于实现此接口的操作上的以下内容：

      ```c++
      class ConcreteOp ... {
      public:
        Value nonStaticMethod(unsigned i);
      };
      ```
    }], "Value", "nonStaticMethodWithParams", (ins "unsigned":$i)
    >,

    StaticInterfaceMethod<[{
      此方法表示一个没有输入且返回类型为 void 的静态接口方法。
      实现此接口的所有操作都必须实现此方法。此方法大致对应于实现此接口的
      操作上的以下内容：

      ```c++
      class ConcreteOp ... {
      public:
        static void staticMethod();
      };
      ```
    }], "void", "staticMethod"
    >,

    StaticInterfaceMethod<[{
      此方法对应于一个具有显式方法体实现的静态接口方法。由于方法体已被
      显式实现，实现此方法的操作不应定义此方法。此方法仅利用操作上已有的
      属性，在本例中为其 `build` 方法。此方法大致对应于接口 `Model` 类上的
      以下内容：

      ```c++
      struct InterfaceTraits {
        /// ... 此处省略 `Concept` 类 ...

        template <typename ConcreteOp>
        struct Model : public Concept {
          Operation *create(OpBuilder &builder, Location loc) const override {
            return ConcreteOp::create(builder, loc);
          }
        }
      };
      ```

      注意，对于实现具有此方法的接口的操作，不需要修改。
    }],
      "Operation *", "create", (ins "OpBuilder &":$builder, "Location":$loc),
      /*methodBody=*/[{
        return ConcreteOp::create(builder, loc);
    }]>,

    InterfaceMethod<[{
      此方法表示一个具有显式方法体实现的非静态方法。由于方法体已被显式
      实现，实现此方法的操作不应定义此方法。此方法仅利用操作上已有的属性，
      在本例中为其 `build` 方法。此方法大致对应于接口 `Model` 类上的以下内容：

      ```c++
      struct InterfaceTraits {
        /// ... 此处省略 `Concept` 类 ...

        template <typename ConcreteOp>
        struct Model : public Concept {
          unsigned getNumInputsAndOutputs(Operation *opaqueOp) const override {
            ConcreteOp op = cast<ConcreteOp>(opaqueOp);
            return op.getNumInputs() + op.getNumOutputs();
          }
        }
      };
      ```

      注意，对于实现具有此方法的接口的操作，不需要修改。
    }],
      "unsigned", "getNumInputsAndOutputs", (ins), /*methodBody=*/[{
        return $_op.getNumInputs() + $_op.getNumOutputs();
    }]>,

    InterfaceMethod<[{
      此方法表示一个具有默认方法体实现的非静态方法。这意味着此处定义的
      实现将放置在附加到实现此接口的每个操作的 trait 类中。这对生成的
      `Concept` 和 `Model` 类没有影响。此方法大致对应于接口 `Trait` 类上的
      以下内容：

      ```c++
      template <typename ConcreteOp>
      class MyTrait : public OpTrait::TraitBase<ConcreteType, MyTrait> {
      public:
        bool isSafeToTransform() {
          ConcreteOp op = cast<ConcreteOp>(this->getOperation());
          return op.getProperties().hasFlag;
        }
      };
      ```

      如 [Traits](Traits) 中所述，由于实现此接口的每个操作也会添加接口 trait，
      此接口上的方法由派生操作继承。这允许将此方法的默认实现注入到实现此接口的
      每个操作中，而不改变接口类本身。如果操作想要覆盖此默认实现，只需实现该
      方法，派生实现将被接口类透明地拾取。

      ```c++
      class ConcreteOp ... {
      public:
        bool isSafeToTransform() {
          // 在此可以覆盖 trait 提供的钩子的默认实现
        }
      };
      ```
    }],
      "bool", "isSafeToTransform", (ins), /*methodBody=*/[{}],
      /*defaultImplementation=*/[{
        return $_op.getProperties().hasFlag;
    }]>,
  ];
}

// 操作接口可以可选地包装在
// `DeclareOpInterfaceMethods` 中。这将导致为成员 `foo`、`bar` 和 `fooStatic`
// 自动生成声明。具有方法体的方法不会在操作声明中声明，而是直接由操作接口
// trait 处理。
def OpWithInferTypeInterfaceOp : Op<...
    [DeclareOpInterfaceMethods<MyInterface>]> { ... }

// 具有默认实现的方法不会生成声明。如果操作希望覆盖默认行为，可以显式
// 指定它希望覆盖的方法。这将强制为这些方法生成声明。
def OpWithOverrideInferTypeInterfaceOp : Op<...
    [DeclareOpInterfaceMethods<MyInterface, ["getNumWithDefault"]>]> { ... }
```

##### 接口继承

接口还支持有限形式的继承，允许以类似于 C++ 等编程语言中类的方式在现有接口之上构建。这更容易构建模块化接口，而不必承受大量显式转型的痛苦。要启用继承，接口只需在其定义中提供所需的基类集合。例如：

```tablegen
def MyBaseInterface : OpInterface<"MyBaseInterface"> {
  ...
}

def MyInterface : OpInterface<"MyInterface", [MyBaseInterface]> {
  ...
}
```

这将导致 `MyInterface` 从 `MyBaseInterface` 继承各种组件，即其接口方法和额外类声明。由于这些继承的组件由不透明的 C++ blob 组成，我们无法正确地沙箱化名称。因此，重要的是确保继承的组件不创建名称重叠，因为这将在接口生成期间导致错误。

`MyInterface` 还将隐式继承 `MyBaseInterface` 上定义的任何基类。但重要的是要注意，对于给定的属性、操作或类型，每个接口只有一个实例。继承的接口方法简单地转发到基础接口实现。这产生了一个总体上更简单的系统，也消除了"菱形继承"的潜在问题。属性/操作/类型上的接口可以被视为构成一个集合，其中每个接口（包括基础接口）在此集合中唯一化，并在需要时在其他地方引用。

当向属性、操作或类型添加具有继承的接口时，所有基础接口也会隐式添加。如果用户愿意，仍然可以手动指定基础接口，例如与 `Declare<Attr|Op|Type>InterfaceMethods` 辅助类一起使用。

如果我们的接口被指定为：

```tablegen
def MyBaseInterface : OpInterface<"MyBaseInterface"> {
  ...
}

def MyOtherBaseInterface : OpInterface<MyOtherBaseInterface, [MyBaseInterface]> {
  ...
}

def MyInterface : OpInterface<"MyInterface", [MyBaseInterface, MyOtherBaseInterface]> {
  ...
}
```

附加了 `MyInterface` 的操作将添加以下接口：

* MyBaseInterface、MyOtherBaseInterface、MyInterface

`MyInterface` 和 `MyOtherBaseInterface` 中来自 `MyBaseInterface` 的方法将转发到该操作的单一唯一实现。

##### 生成

一旦定义了接口，可以使用 mlir-tblgen 的 `--gen-<attr|op|type>-interface-decls` 和 `--gen-<attr|op|type>-interface-defs` 选项生成 C++ 头文件和源文件。请注意，在生成接口时，mlir-tblgen 只会生成在顶层输入 `.td` 文件中定义的接口。这意味着在包含文件中定义的任何接口都不会被考虑用于生成。

注意：在 C++ 中定义的现有操作接口可以通过 `OpInterfaceTrait` 类在 ODS 框架中访问。

#### 操作接口列表

MLIR 包含提供功能的标准接口，这些功能可能在许多不同操作中很常见。以下是一些关键接口的列表，任何方言都可以直接使用。每个接口部分标题的格式如下：

*   `接口类名`
    -   (`C++ 类` -- `ODS 类`（如适用））

##### CallInterfaces
*   `CallOpInterface` - 用于表示类似 'call' 的操作
    -   `CallInterfaceCallable getCallableForCallee()`
    -   `void setCalleeFromCallable(CallInterfaceCallable)`
    -   `ArrayAttr getArgAttrsAttr()`
    -   `ArrayAttr getResAttrsAttr()`
    -   `void setArgAttrsAttr(ArrayAttr)`
    -   `void setResAttrsAttr(ArrayAttr)`
    -   `Attribute removeArgAttrsAttr()`
    -   `Attribute removeResAttrsAttr()`
*   `CallableOpInterface` - 用于表示调用的目标被调用者。
    -   `Region * getCallableRegion()`
    -   `ArrayRef<Type> getArgumentTypes()`
    -   `ArrayRef<Type> getResultTypes()`
    -   `ArrayAttr getArgAttrsAttr()`
    -   `ArrayAttr getResAttrsAttr()`
    -   `void setArgAttrsAttr(ArrayAttr)`
    -   `void setResAttrsAttr(ArrayAttr)`
    -   `Attribute removeArgAttrsAttr()`
    -   `Attribute removeResAttrsAttr()`

##### RegionKindInterfaces

*   `RegionKindInterface` - 用于描述区域的抽象语义。
    -   `RegionKind getRegionKind(unsigned index)` - 返回此操作中给定索引处区域的类型。
        -   RegionKind::Graph - 表示没有控制流语义的图形区域
        -   RegionKind::SSACFG - 表示具有基本块和可达性的
            [SSA 风格控制流](LangRef.md/#control-flow-and-ssacfg-regions) 区域
    -   `hasSSADominance(unsigned index)` - 如果此操作中给定索引处的区域需要支配关系，则返回 true。

##### SymbolInterfaces

*   `SymbolOpInterface` - 用于表示
    [`Symbol`](SymbolsAndSymbolTables.md/#symbol) 操作，这些操作直接驻留在定义
    [`SymbolTable`](SymbolsAndSymbolTables.md/#symbol-table) 的区域内。

*   `SymbolUserOpInterface` - 用于表示引用
    [`Symbol`](SymbolsAndSymbolTables.md/#symbol) 操作的操作。这提供了对符号使用进行安全高效验证的能力，以及额外的功能。

# mlir-rewrite

用于简化重写 `.mlir` 文件的工具。以下讨论了一些内置重写及其用法。

注意：该工具目前仍处于非常早期阶段。早期到与其说是工具，不如说是一个不断增长的有用函数集合：最好的使用方式是在分支上直接修改（注册方言、重写等），例如帮助简化重命名操作、将有用的工具函数上游化、指引其他人迁移，然后最终删除。一旦确实有了有用的部分，应该像 mlir-opt 一样进行重构。

[TOC]

## simple-rename

根据子字符串对给定操作进行重命名。匹配和替换使用 LLVM 的 regex sub 进行，而操作名通过常规字符串比较匹配。例如：

```
mlir-rewrite input.mlir -o output.mlir --simple-rename \
   --simple-rename-op-name="test.concat" --simple-rename-match="axis" \
                                         --simple-rename-replace="bxis"
```

将 `test.concat` 操作对应文本范围内的 `axis` 子字符串替换为 `bxis`。

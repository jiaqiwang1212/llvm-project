class Format<bits<4> val> {
    bits<4> Value = val;
}

def Pseudo : Format<0>;  // 伪指令格式，不会被编码成机器码
def FrmA   : Format<1>;  // 算术格式指令
def FrmL   : Format<2>;  // 内存访问格式指令
def FrmJ   : Format<3>;  // 跳转格式指令
def FrmOther : Format<4>;  // 其它格式指令，暂时未定义
// 0 1 2 3 4 可以理解为代表不同指令格式的ID

// Cpu0架构的通用指令格式
// outs       输出操作数
// ins        输入操作数
// asmstr     汇编打印字符串
// pattern    指令选择匹配规则
// itin       指令调度/流水线信息
// f          指令格式分类
class Cpu0Inst<dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin, Format f>: Instruction{
    field bits<32> Inst;
    Format Form = f;

    let Namespace = "Cpu0";
    // 每条指令占用4字节（32位），因此指令长度为4字节
    let Size = 4;

    bits<8> Opcode = 0;

    let Inst{31-24} = Opcode;

    let OutOperandList = outs;
    let InOperandList = ins;

    let AsmString = asmstr;
    // 指令选择时会使用
    let Pattern = pattern;
    // 指令调度，流水线信息
    let Itinerary = itin;

    bits<4> FormBits = Form.Value;

    let TSFlags{3-0} = FormBits;

    let DecoderNamespace = "Cpu0";

    field bits<32> SoftFail = 0;
}

//===----------------------------------------------------------------------===//
// Format A instruction class in Cpu0 : <|opcode|ra|rb|rc|cx|>
//===----------------------------------------------------------------------===//
class FA<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin> :
    Cpu0Inst<outs, ins, asmstr, pattern, itin, FrmA> {
    bits<4> ra;
    bits<4> rb;
    bits<4> rc;
    bits<12> shamt; // shift amount，移位指令中使用，其他指令中为0

    let Opcode = op;

    let Inst{23-20} = ra;
    let Inst{19-16} = rb;
    let Inst{15-12} = rc;
    let Inst{11-0} = shamt;
}

//===----------------------------------------------------------------------===//
// Format L instruction class in Cpu0 : <|opcode|ra|rb|cx|>
//===----------------------------------------------------------------------===//
class FL<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin> :
    Cpu0Inst<outs, ins, asmstr, pattern, itin, FrmL> {
    bits<4> ra;
    bits<4> rb;
    bits<16> imm16;

    let Opcode = op;

    let Inst{23-20} = ra;
    let Inst{19-16} = rb;
    let Inst{15-0} = imm16;
}

//===----------------------------------------------------------------------===//
// Format J instruction class in Cpu0 : <|opcode|address|>
//===----------------------------------------------------------------------===//
class FJ<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin> :
    Cpu0Inst<outs, ins, asmstr, pattern, itin, FrmJ> {
    bits<24> address;

    let Opcode = op;

    let Inst{23-0} = address;
}
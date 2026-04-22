def simm16 : Operand<i32> {
  let OperandType = "OPERAND_IMMEDIATE";
}

def off11 : Operand<i32> {
  let OperandType = "OPERAND_IMMEDIATE";
}

def brtarget : Operand<OtherVT>;

def TCoreRetFlag : SDNode<"TCoreISD::RET_FLAG", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue]>;

class TCoreInst<dag outs, dag ins, string asmstr, list<dag> pattern = []>
    : Instruction {
  let Namespace = "TCore";
  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
  let Size = 4;
}

class TCorePseudo<dag outs, dag ins, string asmstr, list<dag> pattern = []>
    : TCoreInst<outs, ins, asmstr, pattern> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

let isMoveImm = 1 in
def MOVri : TCoreInst<(outs GPR:$dst), (ins simm16:$imm), "mov $dst, $imm">;

let isMoveReg = 1 in
def MOVrr : TCoreInst<(outs GPR:$dst), (ins GPR:$src), "mov $dst, $src",
                      [(set GPR:$dst, GPR:$src)]>;

class BinRR<string mnemonic, SDPatternOperator OpNode>
    : TCoreInst<(outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs),
                !strconcat(mnemonic, " $dst, $rhs"),
                [(set GPR:$dst, (OpNode GPR:$lhs, GPR:$rhs))]> {
  let Constraints = "$dst = $lhs";
}

class BinRI<string mnemonic, SDPatternOperator OpNode>
    : TCoreInst<(outs GPR:$dst), (ins GPR:$lhs, simm16:$rhs),
                !strconcat(mnemonic, " $dst, $rhs"),
                [(set GPR:$dst, (OpNode GPR:$lhs, imm:$rhs))]> {
  let Constraints = "$dst = $lhs";
}

def ADDrr : BinRR<"add", add>;
def ADDri : BinRI<"add", add>;
def SUBrr : BinRR<"sub", sub>;
def SUBri : BinRI<"sub", sub>;
def MULrr : BinRR<"mul", mul>;
def MULri : BinRI<"mul", mul>;
def XORrr : BinRR<"xor", xor>;
def XORri : BinRI<"xor", xor>;
def ORRrr : BinRR<"orr", or>;
def ORRri : BinRI<"orr", or>;
def ANDrr : BinRR<"and", and>;
def ANDri : BinRI<"and", and>;
def SHLrr : BinRR<"shl", shl>;
def SHLri : BinRI<"shl", shl>;
def SHRrr : BinRR<"shr", srl>;
def SHRri : BinRI<"shr", srl>;

def INC : TCoreInst<(outs GPR:$dst), (ins GPR:$src), "inc $dst"> {
  let Constraints = "$dst = $src";
}

def DEC : TCoreInst<(outs GPR:$dst), (ins GPR:$src), "dec $dst"> {
  let Constraints = "$dst = $src";
}

def NOT : TCoreInst<(outs GPR:$dst), (ins GPR:$src), "not $dst"> {
  let Constraints = "$dst = $src";
}

def CMPrr : TCoreInst<(outs), (ins GPR:$lhs, GPR:$rhs), "cmp $lhs, $rhs">;
def CMPri : TCoreInst<(outs), (ins GPR:$lhs, simm16:$rhs), "cmp $lhs, $rhs">;

def LDUi : TCoreInst<(outs GPR:$dst), (ins simm16:$imm), "ldu $dst, $imm">;
def LDLi : TCoreInst<(outs GPR:$dst), (ins simm16:$imm), "ldl $dst, $imm">;

def LDRri : TCoreInst<(outs GPR:$dst), (ins GPR:$base, off11:$off),
                      "ldr $dst, [$base+$off]">;
def STRri : TCoreInst<(outs), (ins GPR:$src, GPR:$base, off11:$off),
                      "str $src, [$base+$off]">;

let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def BR : TCoreInst<(outs), (ins brtarget:$target), "jmp $target">;

let isBranch = 1, isTerminator = 1 in
def BEQ : TCoreInst<(outs), (ins brtarget:$target), "jeq $target">;
let isBranch = 1, isTerminator = 1 in
def BNE : TCoreInst<(outs), (ins brtarget:$target), "jne $target">;
let isBranch = 1, isTerminator = 1 in
def BGT : TCoreInst<(outs), (ins brtarget:$target), "jgt $target">;
let isBranch = 1, isTerminator = 1 in
def BLT : TCoreInst<(outs), (ins brtarget:$target), "jlt $target">;
let isBranch = 1, isTerminator = 1 in
def BGE : TCoreInst<(outs), (ins brtarget:$target), "jge $target">;
let isBranch = 1, isTerminator = 1 in
def BLE : TCoreInst<(outs), (ins brtarget:$target), "jle $target">;

let isCall = 1, Defs = [LR], Uses = [LR] in
def CALLR : TCoreInst<(outs), (ins GPR:$target), "jmp $target">;

let isReturn = 1, isTerminator = 1, isBarrier = 1 in
def RET : TCorePseudo<(outs), (ins), "jmp lr", [(TCoreRetFlag)]>;

let isPseudo = 1 in
def ADJCALLSTACKDOWN : TCoreInst<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                 "adjsdown $amt1, $amt2">;
let isPseudo = 1 in
def ADJCALLSTACKUP : TCoreInst<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                               "adjsup $amt1, $amt2">;

include "llvm/Target/Target.td"

def TCoreProcGeneric
    : ProcessorModel<"generic", NoSchedModel, []>;

include "TCoreRegisterInfo.td"
include "TCoreCallingConv.td"
include "TCoreInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def TCoreInstrInfo : InstrInfo;

def TCoreInstPrinter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
}

def TCore : Target {
  let InstructionSet = TCoreInstrInfo;
  let AssemblyWriters = [TCoreInstPrinter];
}

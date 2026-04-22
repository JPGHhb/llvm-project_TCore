add_llvm_component_group(TCore)

set(LLVM_TARGET_DEFINITIONS TCore.td)

tablegen(LLVM TCoreGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM TCoreGenCallingConv.inc -gen-callingconv)
tablegen(LLVM TCoreGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM TCoreGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TCoreGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TCoreGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM TCoreGenSDNodeInfo.inc -gen-sd-node-info)

add_public_tablegen_target(TCoreCommonTableGen)

add_llvm_target(TCoreCodeGen
  TCoreAsmPrinter.cpp
  TCoreFrameLowering.cpp
  TCoreInstrInfo.cpp
  TCoreISelDAGToDAG.cpp
  TCoreISelLowering.cpp
  TCoreRegisterInfo.cpp
  TCoreSubtarget.cpp
  TCoreTargetMachine.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  Core
  MC
  SelectionDAG
  Support
  Target
  TargetParser
  TCoreDesc
  TCoreInfo

  ADD_TO_COMPONENT
  TCore
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)

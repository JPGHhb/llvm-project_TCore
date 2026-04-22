add_llvm_component_library(LLVMTCoreDesc
  TCoreAsmBackend.cpp
  TCoreELFObjectWriter.cpp
  TCoreInstPrinter.cpp
  TCoreMCAsmInfo.cpp
  TCoreMCCodeEmitter.cpp
  TCoreMCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  Support
  TCoreInfo

  ADD_TO_COMPONENT
  TCore
)

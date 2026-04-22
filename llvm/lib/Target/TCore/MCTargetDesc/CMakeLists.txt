add_llvm_component_library(LLVMTCoreDesc
  TCoreInstPrinter.cpp
  TCoreMCAsmInfo.cpp
  TCoreMCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  Support
  TCoreInfo

  ADD_TO_COMPONENT
  TCore
)

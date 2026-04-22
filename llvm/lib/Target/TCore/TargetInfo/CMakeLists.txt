add_llvm_component_library(LLVMTCoreInfo
  TCoreTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  TCore
)

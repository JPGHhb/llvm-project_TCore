#include "TargetInfo/TCoreTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheTCoreTarget() {
  static Target TheTCoreTarget;
  return TheTCoreTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTCoreTargetInfo() {
  RegisterTarget<Triple::tcele> X(getTheTCoreTarget(), "tcore",
                                  "TCore Test CPU", "TCORE");
}

#include "TCoreSubtarget.h"

#define DEBUG_TYPE "tcore-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "TCoreGenSubtargetInfo.inc"

using namespace llvm;

TCoreSubtarget::TCoreSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                               const TargetMachine &TM)
    : TCoreGenSubtargetInfo(TT, CPU.empty() ? "generic" : CPU,
                            CPU.empty() ? "generic" : CPU, FS),
      InstrInfo(*this), FrameLowering(), TLInfo(TM, *this) {
  TSInfo = std::make_unique<SelectionDAGTargetInfo>();
  ParseSubtargetFeatures(CPU.empty() ? "generic" : CPU,
                         CPU.empty() ? "generic" : CPU, FS);
}

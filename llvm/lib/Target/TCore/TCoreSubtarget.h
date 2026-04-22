#ifndef LLVM_LIB_TARGET_TCORE_TCORESUBTARGET_H
#define LLVM_LIB_TARGET_TCORE_TCORESUBTARGET_H

#include "TCoreFrameLowering.h"
#include "TCoreISelLowering.h"
#include "TCoreInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "TCoreGenSubtargetInfo.inc"

namespace llvm {

class TCoreSubtarget : public TCoreGenSubtargetInfo {
  TCoreInstrInfo InstrInfo;
  TCoreFrameLowering FrameLowering;
  TCoreTargetLowering TLInfo;
  std::unique_ptr<const SelectionDAGTargetInfo> TSInfo;

public:
  TCoreSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                 const TargetMachine &TM);

  const TCoreInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TCoreRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const TCoreTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return TSInfo.get();
  }

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
};

}

#endif

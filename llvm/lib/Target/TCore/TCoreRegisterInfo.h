#ifndef LLVM_LIB_TARGET_TCORE_TCOREREGISTERINFO_H
#define LLVM_LIB_TARGET_TCORE_TCOREREGISTERINFO_H

#include "TCore.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "TCoreGenRegisterInfo.inc"

namespace llvm {

class TCoreRegisterInfo : public TCoreGenRegisterInfo {
public:
  TCoreRegisterInfo();

  const uint16_t *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;
};

}

#endif

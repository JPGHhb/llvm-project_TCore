#ifndef LLVM_LIB_TARGET_TCORE_TCOREINSTRINFO_H
#define LLVM_LIB_TARGET_TCORE_TCOREINSTRINFO_H

#include "TCoreRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "TCoreGenInstrInfo.inc"

namespace llvm {

class TCoreSubtarget;

class TCoreInstrInfo : public TCoreGenInstrInfo {
  TCoreRegisterInfo RI;

public:
  explicit TCoreInstrInfo(const TCoreSubtarget &STI);

  const TCoreRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator I, Register SrcReg,
                           bool IsKill, int FrameIndex,
                           const TargetRegisterClass *RC, Register VReg,
                           MachineInstr::MIFlag Flags) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator I, Register DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            Register VReg, unsigned SubReg,
                            MachineInstr::MIFlag Flags) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;
};

}

#endif

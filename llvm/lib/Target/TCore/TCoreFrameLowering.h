#ifndef LLVM_LIB_TARGET_TCORE_TCOREFRAMELOWERING_H
#define LLVM_LIB_TARGET_TCORE_TCOREFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class TCoreFrameLowering : public TargetFrameLowering {
public:
  TCoreFrameLowering();

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;
};

}

#endif

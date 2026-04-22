#include "TCoreFrameLowering.h"
#include "TCoreInstrInfo.h"
#include "TCoreSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

TCoreFrameLowering::TCoreFrameLowering()
    : TargetFrameLowering(StackGrowsDown, Align(4), 0, Align(4)) {}

bool TCoreFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  return false;
}

void TCoreFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator I = MBB.begin();
  const auto &TII =
      *static_cast<const TCoreInstrInfo *>(MF.getSubtarget().getInstrInfo());
  int64_t StackSize = MF.getFrameInfo().getStackSize();
  if (!StackSize)
    return;
  BuildMI(MBB, I, DebugLoc(), TII.get(TCore::SUBri), TCore::SP)
      .addReg(TCore::SP)
      .addImm(StackSize);
}

void TCoreFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  const auto &TII =
      *static_cast<const TCoreInstrInfo *>(MF.getSubtarget().getInstrInfo());
  int64_t StackSize = MF.getFrameInfo().getStackSize();
  if (!StackSize)
    return;
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  BuildMI(MBB, I, DebugLoc(), TII.get(TCore::ADDri), TCore::SP)
      .addReg(TCore::SP)
      .addImm(StackSize);
}

MachineBasicBlock::iterator TCoreFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  return MBB.erase(I);
}

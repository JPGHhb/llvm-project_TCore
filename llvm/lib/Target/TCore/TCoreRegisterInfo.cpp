#include "TCoreFrameLowering.h"
#include "TCoreRegisterInfo.h"
#include "TCoreInstrInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "TCoreGenRegisterInfo.inc"

TCoreRegisterInfo::TCoreRegisterInfo() : TCoreGenRegisterInfo(TCore::LR) {}

const uint16_t *
TCoreRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_TCore_SaveList;
}

const uint32_t *
TCoreRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID CC) const {
  return CSR_TCore_RegMask;
}

BitVector TCoreRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(TCore::SP);
  return Reserved;
}

bool TCoreRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const auto &TII = *MF.getSubtarget().getInstrInfo();
  int FI = MI.getOperand(FIOperandNum).getIndex();
  int64_t Offset = MFI.getObjectOffset(FI) + MI.getOperand(FIOperandNum + 1).getImm();
  Offset += MFI.getStackSize();
  if (MI.getOpcode() == TCore::ADDri && FIOperandNum == 1) {
    Register DestReg = MI.getOperand(0).getReg();
    if (DestReg != TCore::SP)
      BuildMI(MBB, II, MI.getDebugLoc(), TII.get(TCore::MOVrr), DestReg)
          .addReg(TCore::SP);
    MI.getOperand(1).ChangeToRegister(DestReg, false);
  } else {
    MI.getOperand(FIOperandNum).ChangeToRegister(TCore::SP, false);
  }
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  return false;
}

Register TCoreRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return TCore::SP;
}

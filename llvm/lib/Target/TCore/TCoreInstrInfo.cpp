#include "TCoreInstrInfo.h"
#include "TCoreSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "TCoreGenInstrInfo.inc"

static bool isCondBranchOpcode(unsigned Opc) {
  switch (Opc) {
  case TCore::BEQ:
  case TCore::BNE:
  case TCore::BGT:
  case TCore::BLT:
  case TCore::BGE:
  case TCore::BLE:
    return true;
  default:
    return false;
  }
}

TCoreInstrInfo::TCoreInstrInfo(const TCoreSubtarget &STI)
    : TCoreGenInstrInfo(STI, RI, TCore::ADJCALLSTACKDOWN,
                        TCore::ADJCALLSTACKUP) {}

void TCoreInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I,
                                 const DebugLoc &DL, Register DestReg,
                                 Register SrcReg, bool KillSrc,
                                 bool RenamableDest,
                                 bool RenamableSrc) const {
  BuildMI(MBB, I, DL, get(TCore::MOVrr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
}

void TCoreInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator I,
                                         Register SrcReg, bool IsKill,
                                         int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         Register VReg,
                                         MachineInstr::MIFlag Flags) const {
  BuildMI(MBB, I, DebugLoc(), get(TCore::STRri))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
}

void TCoreInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator I,
                                          Register DestReg, int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          Register VReg, unsigned SubReg,
                                          MachineInstr::MIFlag Flags) const {
  BuildMI(MBB, I, DebugLoc(), get(TCore::LDRri), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0);
}

unsigned TCoreInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                      int *BytesRemoved) const {
  unsigned Count = 0;
  while (!MBB.empty()) {
    MachineInstr &MI = MBB.back();
    unsigned Opc = MI.getOpcode();
    if (Opc != TCore::BR && Opc != TCore::BEQ && Opc != TCore::BNE &&
        Opc != TCore::BGT && Opc != TCore::BLT && Opc != TCore::BGE &&
        Opc != TCore::BLE)
      break;
    MI.eraseFromParent();
    ++Count;
  }
  if (BytesRemoved)
    *BytesRemoved += static_cast<int>(Count * 4);
  return Count;
}

unsigned TCoreInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                      MachineBasicBlock *TBB,
                                      MachineBasicBlock *FBB,
                                      ArrayRef<MachineOperand> Cond,
                                      const DebugLoc &DL,
                                      int *BytesAdded) const {
  unsigned Count = 0;
  if (Cond.empty()) {
    BuildMI(&MBB, DL, get(TCore::BR)).addMBB(TBB);
    Count = 1;
  } else {
    unsigned Opc = static_cast<unsigned>(Cond[0].getImm());
    BuildMI(&MBB, DL, get(Opc)).addMBB(TBB);
    Count = 1;
    if (FBB) {
      BuildMI(&MBB, DL, get(TCore::BR)).addMBB(FBB);
      ++Count;
    }
  }
  if (BytesAdded)
    *BytesAdded += static_cast<int>(Count * 4);
  return Count;
}

bool TCoreInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const {
  TBB = FBB = nullptr;
  Cond.clear();

  if (MBB.empty())
    return false;

  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end() || !I->isTerminator())
    return false;

  MachineInstr *Last = &*I;
  MachineInstr *SecondLast = nullptr;
  if (I != MBB.begin()) {
    --I;
    while (true) {
      if (!I->isDebugInstr()) {
        SecondLast = &*I;
        break;
      }
      if (I == MBB.begin())
        break;
      --I;
    }
  }

  if (Last->getOpcode() == TCore::BR) {
    TBB = Last->getOperand(0).getMBB();
    if (SecondLast && isCondBranchOpcode(SecondLast->getOpcode())) {
      FBB = TBB;
      TBB = SecondLast->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(SecondLast->getOpcode()));
    }
    return false;
  }

  if (isCondBranchOpcode(Last->getOpcode())) {
    TBB = Last->getOperand(0).getMBB();
    Cond.push_back(MachineOperand::CreateImm(Last->getOpcode()));
    return false;
  }

  return true;
}

bool TCoreInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  if (Cond.size() != 1 || !Cond[0].isImm())
    return true;

  unsigned Opc = static_cast<unsigned>(Cond[0].getImm());
  switch (Opc) {
  case TCore::BEQ:
    Cond[0].setImm(TCore::BNE);
    return false;
  case TCore::BNE:
    Cond[0].setImm(TCore::BEQ);
    return false;
  case TCore::BGT:
    Cond[0].setImm(TCore::BLE);
    return false;
  case TCore::BLE:
    Cond[0].setImm(TCore::BGT);
    return false;
  case TCore::BLT:
    Cond[0].setImm(TCore::BGE);
    return false;
  case TCore::BGE:
    Cond[0].setImm(TCore::BLT);
    return false;
  default:
    return true;
  }
}

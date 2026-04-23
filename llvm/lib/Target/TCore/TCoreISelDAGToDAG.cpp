#include "TCore.h"
#include "TCoreTargetMachine.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"

using namespace llvm;

#define DEBUG_TYPE "tcore-isel"

namespace {

class TCoreDAGToDAGISel : public SelectionDAGISel {
public:
  explicit TCoreDAGToDAGISel(TCoreTargetMachine &TM, CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  void Select(SDNode *Node) override;
  bool selectFrameIndexAddr(SDValue Addr, SDValue &Base, SDValue &Offset);
  bool selectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);

#include "TCoreGenDAGISel.inc"
};

class TCoreDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;

  explicit TCoreDAGToDAGISelLegacy(TCoreTargetMachine &TM,
                                   CodeGenOptLevel OptLevel)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<TCoreDAGToDAGISel>(TM, OptLevel)) {}

  StringRef getPassName() const override {
    return "TCore DAG->DAG Pattern Instruction Selection";
  }
};

} // namespace

char TCoreDAGToDAGISelLegacy::ID = 0;

FunctionPass *llvm::createTCoreISelDag(TCoreTargetMachine &TM,
                                       CodeGenOptLevel OptLevel) {
  return new TCoreDAGToDAGISelLegacy(TM, OptLevel);
}

bool TCoreDAGToDAGISel::selectFrameIndexAddr(SDValue Addr, SDValue &Base,
                                             SDValue &Offset) {
  if (auto *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
    Offset = CurDAG->getTargetConstant(0, SDLoc(Addr), MVT::i32);
    return true;
  }
  return false;
}

bool TCoreDAGToDAGISel::selectAddr(SDValue Addr, SDValue &Base,
                                   SDValue &Offset) {
  if (selectFrameIndexAddr(Addr, Base, Offset))
    return true;

  if (Addr.getOpcode() == ISD::ADD) {
    if (auto *CN = dyn_cast<ConstantSDNode>(Addr.getOperand(1))) {
      int64_t Imm = CN->getSExtValue();
      if (isInt<11>(Imm)) {
        Base = Addr.getOperand(0);
        Offset = CurDAG->getTargetConstant(Imm, SDLoc(Addr), MVT::i32);
        return true;
      }
    }
    if (auto *CN = dyn_cast<ConstantSDNode>(Addr.getOperand(0))) {
      int64_t Imm = CN->getSExtValue();
      if (isInt<11>(Imm)) {
        Base = Addr.getOperand(1);
        Offset = CurDAG->getTargetConstant(Imm, SDLoc(Addr), MVT::i32);
        return true;
      }
    }
  }

  if (Addr.getValueType() == MVT::i32) {
    Base = Addr;
    Offset = CurDAG->getTargetConstant(0, SDLoc(Addr), MVT::i32);
    return true;
  }

  return false;
}

void TCoreDAGToDAGISel::Select(SDNode *Node) {
  if (Node->isMachineOpcode()) {
    Node->setNodeId(-1);
    return;
  }

  SDLoc DL(Node);
  switch (Node->getOpcode()) {
  case ISD::Constant: {
    auto *CN = cast<ConstantSDNode>(Node);
    if (Node->getValueType(0) == MVT::i32) {
      int64_t Imm = CN->getSExtValue();
      if (!isInt<16>(Imm) && !isUInt<16>(Imm)) {
        SDValue TImm = CurDAG->getTargetConstant(CN->getZExtValue(), DL, MVT::i32);
        ReplaceNode(Node, CurDAG->getMachineNode(TCore::MOVi32, DL, MVT::i32,
                                                 TImm));
        return;
      }
    }
    break;
  }
  case ISD::FrameIndex: {
    int FI = cast<FrameIndexSDNode>(Node)->getIndex();
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i32);
    SDValue Zero = CurDAG->getTargetConstant(0, DL, MVT::i32);
    ReplaceNode(Node,
                CurDAG->getMachineNode(TCore::ADDri, DL, MVT::i32, TFI, Zero));
    return;
  }
  case ISD::LOAD: {
    auto *LD = cast<LoadSDNode>(Node);
    SDValue Base;
    SDValue Offset;
    if (!selectAddr(LD->getBasePtr(), Base, Offset))
      break;

    SDVTList VTs = CurDAG->getVTList(LD->getMemoryVT(), MVT::Other);
    SDValue Ops[] = {Base, Offset, LD->getChain()};
    auto *Load = CurDAG->getMachineNode(TCore::LDRri, DL, VTs, Ops);
    CurDAG->setNodeMemRefs(cast<MachineSDNode>(Load), {LD->getMemOperand()});
    ReplaceNode(Node, Load);
    return;
  }
  case ISD::STORE: {
    auto *ST = cast<StoreSDNode>(Node);
    SDValue Base;
    SDValue Offset;
    if (!selectAddr(ST->getBasePtr(), Base, Offset))
      break;

    SDValue Ops[] = {ST->getValue(), Base, Offset, ST->getChain()};
    auto *Store =
        CurDAG->getMachineNode(TCore::STRri, DL, MVT::Other, Ops);
    CurDAG->setNodeMemRefs(cast<MachineSDNode>(Store), {ST->getMemOperand()});
    ReplaceNode(Node, Store);
    return;
  }
  case ISD::BR: {
    auto *BB = cast<BasicBlockSDNode>(Node->getOperand(1));
    SDValue Dest = CurDAG->getBasicBlock(BB->getBasicBlock());
    auto *Br =
        CurDAG->getMachineNode(TCore::BR, DL, MVT::Other, Dest,
                               Node->getOperand(0));
    ReplaceNode(Node, Br);
    return;
  }
  case ISD::BR_CC: {
    ISD::CondCode CC = cast<CondCodeSDNode>(Node->getOperand(1))->get();
    unsigned BranchOpc;
    switch (CC) {
    case ISD::SETEQ:
      BranchOpc = TCore::BEQ;
      break;
    case ISD::SETNE:
      BranchOpc = TCore::BNE;
      break;
    case ISD::SETGT:
      BranchOpc = TCore::BGT;
      break;
    case ISD::SETLT:
      BranchOpc = TCore::BLT;
      break;
    case ISD::SETGE:
      BranchOpc = TCore::BGE;
      break;
    case ISD::SETLE:
      BranchOpc = TCore::BLE;
      break;
    default:
      break;
    }

    if (CC != ISD::SETEQ && CC != ISD::SETNE && CC != ISD::SETGT &&
        CC != ISD::SETLT && CC != ISD::SETGE && CC != ISD::SETLE)
      break;

    SDValue Chain = Node->getOperand(0);
    SDValue LHS = Node->getOperand(2);
    SDValue RHS = Node->getOperand(3);
    auto *BB = cast<BasicBlockSDNode>(Node->getOperand(4));
    SDValue Dest = CurDAG->getBasicBlock(BB->getBasicBlock());

    MachineSDNode *Cmp = nullptr;
    if (auto *RHSC = dyn_cast<ConstantSDNode>(RHS)) {
      int64_t ImmVal = RHSC->getSExtValue();
      if (isInt<16>(ImmVal) || isUInt<16>(ImmVal)) {
        SDValue Imm = CurDAG->getTargetConstant(ImmVal, DL, MVT::i32);
        Cmp = CurDAG->getMachineNode(TCore::CMPri, DL, MVT::Other, LHS, Imm,
                                     Chain);
      } else {
        SDValue LargeImm = CurDAG->getTargetConstant(RHSC->getZExtValue(), DL,
                                                     MVT::i32);
        auto *Mov =
            CurDAG->getMachineNode(TCore::MOVi32, DL, MVT::i32, LargeImm);
        Cmp = CurDAG->getMachineNode(TCore::CMPrr, DL, MVT::Other, LHS,
                                     SDValue(Mov, 0), Chain);
      }
    } else {
      Cmp = CurDAG->getMachineNode(TCore::CMPrr, DL, MVT::Other, LHS, RHS,
                                   Chain);
    }

    auto *Br = CurDAG->getMachineNode(BranchOpc, DL, MVT::Other,
                                      Dest, SDValue(Cmp, 0));
    ReplaceNode(Node, Br);
    return;
  }
  default:
    break;
  }

  SelectCode(Node);
}

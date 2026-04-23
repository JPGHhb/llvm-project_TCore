#include "TCoreISelLowering.h"
#include "TCoreSubtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#include "TCoreGenCallingConv.inc"

static SDValue lowerCallResult(SDValue Chain, SDValue Glue,
                               const SmallVectorImpl<CCValAssign> &RVLocs,
                               SDLoc DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) {
  for (const CCValAssign &VA : RVLocs) {
    if (!VA.isRegLoc())
      report_fatal_error("TCore call result stack locations not implemented");

    SDValue RetValue =
        DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getValVT(), Glue);
    Chain = RetValue.getValue(1);
    Glue = RetValue.getValue(2);
    InVals.push_back(RetValue);
  }

  return Chain;
}

TCoreTargetLowering::TCoreTargetLowering(const TargetMachine &TM,
                                         const TCoreSubtarget &STI)
    : TargetLowering(TM, STI) {
  addRegisterClass(MVT::i32, &TCore::GPRRegClass);
  setStackPointerRegisterToSaveRestore(TCore::SP);
  setBooleanContents(ZeroOrOneBooleanContent);
  computeRegisterProperties(STI.getRegisterInfo());
}

const char *TCoreTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case TCoreISD::RET_FLAG:
    return "TCoreISD::RET_FLAG";
  case TCoreISD::CALL:
    return "TCoreISD::CALL";
  case TCoreISD::CALL_REG:
    return "TCoreISD::CALL_REG";
  default:
    return nullptr;
  }
}

SDValue TCoreTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  SmallVector<CCValAssign, 8> ArgLocs;
  SmallVector<SDValue, 8> ArgChains;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_TCore);

  for (const CCValAssign &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      Register VReg = RegInfo.createVirtualRegister(&TCore::GPRRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, VA.getValVT());
      InVals.push_back(ArgIn);
      ArgChains.push_back(ArgIn.getValue(1));
    } else {
      int FI = MF.getFrameInfo().CreateFixedObject(4, VA.getLocMemOffset(), true);
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue Load = DAG.getLoad(VA.getValVT(), DL, Chain, FIN,
                                 MachinePointerInfo::getFixedStack(
                                     DAG.getMachineFunction(), FI));
      InVals.push_back(Load);
      ArgChains.push_back(Load.getValue(1));
    }
  }

  if (!ArgChains.empty())
    Chain = DAG.getTokenFactor(DL, ArgChains);

  return Chain;
}

SDValue TCoreTargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
    SelectionDAG &DAG) const {
  SmallVector<CCValAssign, 8> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_TCore);

  SDValue Glue;
  for (unsigned I = 0; I < RVLocs.size(); ++I) {
    Chain = DAG.getCopyToReg(Chain, DL, RVLocs[I].getLocReg(), OutVals[I], Glue);
    Glue = Chain.getValue(1);
  }

  if (Glue)
    return DAG.getNode(TCoreISD::RET_FLAG, DL, MVT::Other, Chain, Glue);
  return DAG.getNode(TCoreISD::RET_FLAG, DL, MVT::Other, Chain);
}

SDValue TCoreTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                       SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;
  bool &IsTailCall = CLI.IsTailCall;

  IsTailCall = false;

  SmallVector<CCValAssign, 8> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_TCore);

  SmallVector<CCValAssign, 8> RVLocs;
  CCState RetCCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                    *DAG.getContext());
  RetCCInfo.AllocateStack(CCInfo.getStackSize(), Align(4));
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_TCore);

  unsigned NumBytes = RetCCInfo.getStackSize();
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  SDValue StackPtr;

  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    CCValAssign &VA = ArgLocs[I];
    SDValue Arg = OutVals[I];

    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, DL, VA.getLocVT(), Arg);
      break;
    }

    if (VA.isRegLoc()) {
      RegsToPass.push_back({VA.getLocReg(), Arg});
      continue;
    }

    if (!StackPtr.getNode())
      StackPtr = DAG.getCopyFromReg(Chain, DL, TCore::SP,
                                    getPointerTy(DAG.getDataLayout()));
    SDValue Offset = DAG.getIntPtrConstant(VA.getLocMemOffset(), DL);
    SDValue PtrOff =
        DAG.getNode(ISD::ADD, DL, getPointerTy(DAG.getDataLayout()), StackPtr,
                    Offset);
    MemOpChains.push_back(
        DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo()));
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue Glue;
  for (auto [PhysReg, Arg] : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, PhysReg, Arg, Glue);
    Glue = Chain.getValue(1);
  }

  bool IsDirect = true;
  if (auto *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, MVT::i32);
  else if (auto *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);
  else
    IsDirect = false;

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);
  for (auto [PhysReg, Arg] : RegsToPass)
    Ops.push_back(DAG.getRegister(PhysReg, Arg.getValueType()));

  const auto &STI = DAG.getMachineFunction().getSubtarget<TCoreSubtarget>();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);
  assert(Mask && "Missing call preserved mask");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (Glue.getNode())
    Ops.push_back(Glue);

  Chain = DAG.getNode(IsDirect ? TCoreISD::CALL : TCoreISD::CALL_REG, DL,
                      NodeTys, Ops);
  Glue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  return lowerCallResult(Chain, Glue, RVLocs, DL, DAG, InVals);
}

SDValue TCoreTargetLowering::LowerOperation(SDValue Op,
                                            SelectionDAG &DAG) const {
  report_fatal_error(
      "TCore custom lowering not implemented for this operation yet");
}

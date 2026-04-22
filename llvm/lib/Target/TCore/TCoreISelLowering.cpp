#include "TCoreISelLowering.h"
#include "TCoreSubtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#include "TCoreGenCallingConv.inc"

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
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_TCore);

  for (const CCValAssign &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      Register VReg = RegInfo.createVirtualRegister(&TCore::GPRRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, VA.getValVT());
      InVals.push_back(ArgIn);
    } else {
      int FI = MF.getFrameInfo().CreateFixedObject(4, VA.getLocMemOffset(), true);
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue Load = DAG.getLoad(VA.getValVT(), DL, Chain, FIN,
                                 MachinePointerInfo::getFixedStack(
                                     DAG.getMachineFunction(), FI));
      InVals.push_back(Load);
    }
  }
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

SDValue TCoreTargetLowering::LowerOperation(SDValue Op,
                                            SelectionDAG &DAG) const {
  report_fatal_error(
      "TCore custom lowering not implemented for this operation yet");
}

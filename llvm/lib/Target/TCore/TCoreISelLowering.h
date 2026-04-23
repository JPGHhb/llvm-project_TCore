#ifndef LLVM_LIB_TARGET_TCORE_TCOREISELLOWERING_H
#define LLVM_LIB_TARGET_TCORE_TCOREISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class TCoreSubtarget;
class TCoreTargetMachine;

namespace TCoreISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_FLAG,
  CALL,
  CALL_REG
};
}

class TCoreTargetLowering : public TargetLowering {
public:
  explicit TCoreTargetLowering(const TargetMachine &TM,
                               const TCoreSubtarget &STI);

  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  SDValue LowerCall(CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
};

}

#endif

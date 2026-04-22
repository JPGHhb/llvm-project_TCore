#include "TCore.h"
#include "TCoreTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"

using namespace llvm;

#define DEBUG_TYPE "tcore-isel"

namespace {

class TCoreDAGToDAGISel : public SelectionDAGISel {
public:
  explicit TCoreDAGToDAGISel(TCoreTargetMachine &TM, CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  void Select(SDNode *Node) override;

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

void TCoreDAGToDAGISel::Select(SDNode *Node) {
  if (Node->isMachineOpcode()) {
    Node->setNodeId(-1);
    return;
  }

  SelectCode(Node);
}

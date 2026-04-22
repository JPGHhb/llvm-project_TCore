#include "MCTargetDesc/TCoreInstPrinter.h"
#include "TargetInfo/TCoreTargetInfo.h"
#include "TCore.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

namespace {
class TCoreAsmPrinter : public AsmPrinter {
public:
  static char ID;

  explicit TCoreAsmPrinter(TargetMachine &TM,
                           std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID) {}

  StringRef getPassName() const override { return "TCore Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override {
    switch (MI->getOpcode()) {
    case TCore::ADJCALLSTACKDOWN:
    case TCore::ADJCALLSTACKUP:
      return;
    case TCore::RET: {
      MCInst RetInst;
      RetInst.setOpcode(TCore::CALLR);
      RetInst.addOperand(MCOperand::createReg(TCore::LR));
      EmitToStreamer(*OutStreamer, RetInst);
      return;
    }
    default:
      break;
    }

    MCInst OutMI;
    OutMI.setOpcode(MI->getOpcode());
    for (const MachineOperand &MO : MI->operands()) {
      if (MO.isReg()) {
        if (MO.getReg())
          OutMI.addOperand(MCOperand::createReg(MO.getReg()));
      } else if (MO.isImm()) {
        OutMI.addOperand(MCOperand::createImm(MO.getImm()));
      } else if (MO.isMBB()) {
        OutMI.addOperand(
            MCOperand::createExpr(MCSymbolRefExpr::create(MO.getMBB()->getSymbol(),
                                                          OutContext)));
      } else if (MO.isGlobal()) {
        OutMI.addOperand(MCOperand::createExpr(
            MCSymbolRefExpr::create(getSymbol(MO.getGlobal()), OutContext)));
      }
    }
    EmitToStreamer(*OutStreamer, OutMI);
  }
};
}

char TCoreAsmPrinter::ID = 0;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTCoreAsmPrinter() {
  RegisterAsmPrinter<TCoreAsmPrinter> X(getTheTCoreTarget());
}

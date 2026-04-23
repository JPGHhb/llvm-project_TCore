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
    case TCore::MOVaddr: {
      const MachineOperand &Target = MI->getOperand(1);
      const MCSymbol *Symbol = nullptr;
      if (Target.isGlobal())
        Symbol = getSymbol(Target.getGlobal());
      else if (Target.isSymbol())
        Symbol = GetExternalSymbolSymbol(Target.getSymbolName());
      else
        report_fatal_error("TCore MOVaddr expects global or external symbol");

      const MCExpr *TargetExpr = MCSymbolRefExpr::create(Symbol, OutContext);
      MCRegister DestReg = MI->getOperand(0).getReg();

      MCInst HiInst;
      HiInst.setOpcode(TCore::LDUi);
      HiInst.addOperand(MCOperand::createReg(DestReg));
      HiInst.addOperand(MCOperand::createExpr(TargetExpr));
      EmitToStreamer(*OutStreamer, HiInst);

      MCInst LoInst;
      LoInst.setOpcode(TCore::LDLi);
      LoInst.addOperand(MCOperand::createReg(DestReg));
      LoInst.addOperand(MCOperand::createExpr(TargetExpr));
      EmitToStreamer(*OutStreamer, LoInst);
      return;
    }
    case TCore::MOVi32: {
      uint32_t Value = static_cast<uint32_t>(MI->getOperand(1).getImm());
      uint32_t Hi = Value >> 16;
      uint32_t Lo = Value & 0xffffu;
      MCRegister DestReg = MI->getOperand(0).getReg();

      MCInst HiInst;
      HiInst.setOpcode(TCore::LDUi);
      HiInst.addOperand(MCOperand::createReg(DestReg));
      HiInst.addOperand(MCOperand::createImm(Hi));
      EmitToStreamer(*OutStreamer, HiInst);

      MCInst LoInst;
      LoInst.setOpcode(TCore::LDLi);
      LoInst.addOperand(MCOperand::createReg(DestReg));
      LoInst.addOperand(MCOperand::createImm(Lo));
      EmitToStreamer(*OutStreamer, LoInst);
      return;
    }
    case TCore::CALL: {
      const MachineOperand &Target = MI->getOperand(0);
      const MCSymbol *Symbol = nullptr;
      if (Target.isGlobal())
        Symbol = getSymbol(Target.getGlobal());
      else if (Target.isSymbol())
        Symbol = GetExternalSymbolSymbol(Target.getSymbolName());
      else
        report_fatal_error("TCore CALL expects global or external symbol");

      const MCExpr *TargetExpr = MCSymbolRefExpr::create(Symbol, OutContext);

      MCInst HiInst;
      HiInst.setOpcode(TCore::LDUi);
      HiInst.addOperand(MCOperand::createReg(TCore::R14));
      HiInst.addOperand(MCOperand::createExpr(TargetExpr));
      EmitToStreamer(*OutStreamer, HiInst);

      MCInst LoInst;
      LoInst.setOpcode(TCore::LDLi);
      LoInst.addOperand(MCOperand::createReg(TCore::R14));
      LoInst.addOperand(MCOperand::createExpr(TargetExpr));
      EmitToStreamer(*OutStreamer, LoInst);

      MCInst CallInst;
      CallInst.setOpcode(TCore::CALLR);
      CallInst.addOperand(MCOperand::createReg(TCore::R14));
      EmitToStreamer(*OutStreamer, CallInst);
      return;
    }
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

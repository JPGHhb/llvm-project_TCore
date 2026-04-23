#include "MCTargetDesc/TCoreFixupKinds.h"
#include "MCTargetDesc/TCoreMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {

class TCoreMCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;

public:
  explicit TCoreMCCodeEmitter(MCContext &Ctx) : Ctx(Ctx) {}

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

private:
  static constexpr uint32_t OpCodeShift = 27;
  static constexpr uint32_t Arg0Shift = 22;
  static constexpr uint32_t Arg1IsRegShift = 21;
  static constexpr uint32_t OffsetShift = 10;
  static constexpr uint32_t PayloadShift = 5;

  static constexpr uint32_t RegMask = 0x1f;
  static constexpr uint32_t Imm16Mask = 0xffff;
  static constexpr uint32_t Off11Mask = 0x7ff;

  static uint32_t getRawOpcode(unsigned Opcode);
  uint32_t getRegEncoding(MCRegister Reg) const;
  uint32_t getJumpImmediateEncoding(const MCInst &MI, unsigned OpNo,
                                    SmallVectorImpl<MCFixup> &Fixups) const;
  uint32_t getImm16Encoding(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            MCFixupKind Kind = TCore::fixup_tcore_abs16) const;
  uint32_t encodeRegForm(uint32_t RawOpcode, uint32_t Arg0, uint32_t Arg1Reg,
                         int32_t Offset = 0) const;
  uint32_t encodeImmForm(uint32_t RawOpcode, uint32_t Arg0,
                         uint32_t Imm16) const;
};

} // namespace

MCCodeEmitter *llvm::createTCoreMCCodeEmitter(const MCInstrInfo &,
                                              MCContext &Ctx) {
  return new TCoreMCCodeEmitter(Ctx);
}

static void addFixup(SmallVectorImpl<MCFixup> &Fixups, const MCExpr *Value,
                     MCFixupKind Kind, bool IsPCRel = false) {
  Fixups.push_back(MCFixup::create(0, Value, Kind, IsPCRel));
}

uint32_t TCoreMCCodeEmitter::getRawOpcode(unsigned Opcode) {
  switch (Opcode) {
  case TCore::MOVri:
  case TCore::MOVrr:
    return 0x13;
  case TCore::ADDri:
  case TCore::ADDrr:
    return 0x07;
  case TCore::SUBri:
  case TCore::SUBrr:
    return 0x08;
  case TCore::MULri:
  case TCore::MULrr:
    return 0x09;
  case TCore::XORri:
  case TCore::XORrr:
    return 0x0c;
  case TCore::ORRri:
  case TCore::ORRrr:
    return 0x0d;
  case TCore::ANDri:
  case TCore::ANDrr:
    return 0x0e;
  case TCore::SHLri:
  case TCore::SHLrr:
    return 0x10;
  case TCore::SHRri:
  case TCore::SHRrr:
    return 0x11;
  case TCore::CMPri:
  case TCore::CMPrr:
    return 0x12;
  case TCore::INC:
    return 0x0a;
  case TCore::DEC:
    return 0x0b;
  case TCore::NOT:
    return 0x0f;
  case TCore::LDRri:
    return 0x14;
  case TCore::LDUi:
    return 0x15;
  case TCore::LDLi:
    return 0x16;
  case TCore::STRri:
    return 0x17;
  case TCore::BR:
  case TCore::CALLR:
    return 0x00;
  case TCore::BEQ:
    return 0x01;
  case TCore::BNE:
    return 0x02;
  case TCore::BGT:
    return 0x03;
  case TCore::BLT:
    return 0x04;
  case TCore::BGE:
    return 0x05;
  case TCore::BLE:
    return 0x06;
  default:
    report_fatal_error("TCoreMCCodeEmitter: unsupported opcode");
  }
}

uint32_t TCoreMCCodeEmitter::getRegEncoding(MCRegister Reg) const {
  return Ctx.getRegisterInfo()->getEncodingValue(Reg) & RegMask;
}

uint32_t TCoreMCCodeEmitter::getImm16Encoding(const MCInst &MI, unsigned OpNo,
                                              SmallVectorImpl<MCFixup> &Fixups,
                                              MCFixupKind Kind) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return static_cast<uint32_t>(MO.getImm()) & Imm16Mask;

  assert(MO.isExpr() && "expected immediate or expression operand");
  addFixup(Fixups, MO.getExpr(), Kind);
  return 0;
}

uint32_t TCoreMCCodeEmitter::getJumpImmediateEncoding(
    const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return static_cast<uint32_t>(MO.getImm()) & Imm16Mask;

  assert(MO.isExpr() && "expected immediate or expression jump target");
  addFixup(Fixups, MO.getExpr(), TCore::fixup_tcore_pcrel_16word, true);
  return 0;
}

uint32_t TCoreMCCodeEmitter::encodeRegForm(uint32_t RawOpcode, uint32_t Arg0,
                                           uint32_t Arg1Reg,
                                           int32_t Offset) const {
  uint32_t Bits = 0;
  Bits |= (RawOpcode & RegMask) << OpCodeShift;
  Bits |= (Arg0 & RegMask) << Arg0Shift;
  Bits |= 1u << Arg1IsRegShift;
  Bits |= (static_cast<uint32_t>(Offset) & Off11Mask) << OffsetShift;
  Bits |= (Arg1Reg & RegMask) << PayloadShift;
  return Bits;
}

uint32_t TCoreMCCodeEmitter::encodeImmForm(uint32_t RawOpcode, uint32_t Arg0,
                                           uint32_t Imm16) const {
  uint32_t Bits = 0;
  Bits |= (RawOpcode & RegMask) << OpCodeShift;
  Bits |= (Arg0 & RegMask) << Arg0Shift;
  Bits |= (Imm16 & Imm16Mask) << PayloadShift;
  return Bits;
}

void TCoreMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                           SmallVectorImpl<char> &CB,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &) const {
  const unsigned Opcode = MI.getOpcode();
  const uint32_t RawOpcode = getRawOpcode(Opcode);
  uint32_t Bits = 0;

  switch (Opcode) {
  case TCore::MOVri:
    Bits = encodeImmForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getImm16Encoding(MI, 1, Fixups));
    break;
  case TCore::ADDri:
  case TCore::SUBri:
  case TCore::MULri:
  case TCore::XORri:
  case TCore::ORRri:
  case TCore::ANDri:
  case TCore::SHLri:
  case TCore::SHRri:
    Bits = encodeImmForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getImm16Encoding(MI, 2, Fixups));
    break;
  case TCore::CMPri:
    Bits = encodeImmForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getImm16Encoding(MI, 1, Fixups));
    break;
  case TCore::LDUi:
    Bits = encodeImmForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getImm16Encoding(MI, 1, Fixups,
                                          TCore::fixup_tcore_hi16));
    break;
  case TCore::LDLi:
    Bits = encodeImmForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getImm16Encoding(MI, 1, Fixups,
                                          TCore::fixup_tcore_lo16));
    break;
  case TCore::MOVrr:
    Bits = encodeRegForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getRegEncoding(MI.getOperand(1).getReg()));
    break;
  case TCore::ADDrr:
  case TCore::SUBrr:
  case TCore::MULrr:
  case TCore::XORrr:
  case TCore::ORRrr:
  case TCore::ANDrr:
  case TCore::SHLrr:
  case TCore::SHRrr:
    Bits = encodeRegForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getRegEncoding(MI.getOperand(2).getReg()));
    break;
  case TCore::CMPrr:
    Bits = encodeRegForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getRegEncoding(MI.getOperand(1).getReg()));
    break;
  case TCore::INC:
  case TCore::DEC:
  case TCore::NOT:
    Bits = encodeRegForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         0);
    break;
  case TCore::LDRri: {
    const int64_t Offset = MI.getOperand(2).getImm();
    if (!isInt<11>(Offset))
      report_fatal_error("TCoreMCCodeEmitter: ldr offset out of range");
    Bits = encodeRegForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getRegEncoding(MI.getOperand(1).getReg()),
                         static_cast<int32_t>(Offset));
    break;
  }
  case TCore::STRri: {
    const int64_t Offset = MI.getOperand(2).getImm();
    if (!isInt<11>(Offset))
      report_fatal_error("TCoreMCCodeEmitter: str offset out of range");
    Bits = encodeRegForm(RawOpcode, getRegEncoding(MI.getOperand(0).getReg()),
                         getRegEncoding(MI.getOperand(1).getReg()),
                         static_cast<int32_t>(Offset));
    break;
  }
  case TCore::BR:
  case TCore::BEQ:
  case TCore::BNE:
  case TCore::BGT:
  case TCore::BLT:
  case TCore::BGE:
  case TCore::BLE:
    Bits = encodeImmForm(RawOpcode, 0, getJumpImmediateEncoding(MI, 0, Fixups));
    break;
  case TCore::CALLR:
    Bits = encodeRegForm(RawOpcode, 0, getRegEncoding(MI.getOperand(0).getReg()));
    break;
  default:
    report_fatal_error("TCoreMCCodeEmitter: cannot encode pseudo instruction");
  }

  support::endian::write<uint32_t>(CB, Bits, llvm::endianness::little);
}

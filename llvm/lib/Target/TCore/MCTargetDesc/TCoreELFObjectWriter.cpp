#include "MCTargetDesc/TCoreFixupKinds.h"
#include "MCTargetDesc/TCoreMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"

using namespace llvm;

namespace {

enum TCoreRelocs : unsigned {
  R_TCORE_NONE = 0,
  R_TCORE_32 = 1,
  R_TCORE_PCREL16_WORD = 2,
  R_TCORE_HI16 = 3,
  R_TCORE_LO16 = 4,
  R_TCORE_ABS16 = 5,
};

class TCoreELFObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit TCoreELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(false, OSABI, ELF::EM_TCORE, true) {}

  unsigned getRelocType(const MCFixup &Fixup, const MCValue &,
                        bool IsPCRel) const override {
    switch (Fixup.getKind()) {
    case FK_Data_4:
      return R_TCORE_32;
    case TCore::fixup_tcore_abs16:
      return R_TCORE_ABS16;
    case TCore::fixup_tcore_hi16:
      return R_TCORE_HI16;
    case TCore::fixup_tcore_lo16:
      return R_TCORE_LO16;
    case TCore::fixup_tcore_pcrel_16word:
      return R_TCORE_PCREL16_WORD;
    default:
      return IsPCRel ? R_TCORE_PCREL16_WORD : R_TCORE_NONE;
    }
  }
};

} // namespace

std::unique_ptr<MCObjectTargetWriter>
llvm::createTCoreELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<TCoreELFObjectWriter>(OSABI);
}

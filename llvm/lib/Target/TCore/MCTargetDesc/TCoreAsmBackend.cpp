#include "MCTargetDesc/TCoreFixupKinds.h"
#include "MCTargetDesc/TCoreMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class TCoreAsmBackend : public MCAsmBackend {
  uint8_t OSABI;

public:
  explicit TCoreAsmBackend(uint8_t OSABI)
      : MCAsmBackend(llvm::endianness::little), OSABI(OSABI) {}

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;
  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;
  std::unique_ptr<MCObjectTargetWriter> createObjectTargetWriter() const override;
  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;
};

} // namespace

static uint32_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {
  switch (Fixup.getKind()) {
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    return static_cast<uint32_t>(Value);
  case TCore::fixup_tcore_abs16:
  case TCore::fixup_tcore_lo16:
    return static_cast<uint32_t>(Value) & 0xffffu;
  case TCore::fixup_tcore_hi16:
    return (static_cast<uint64_t>(Value) >> 16) & 0xffffu;
  case TCore::fixup_tcore_pcrel_16word: {
    const int64_t SignedValue = static_cast<int64_t>(Value);
    if ((SignedValue & 0x3) != 0)
      Ctx.reportError(Fixup.getLoc(),
                      "TCore branch target must be word aligned");
    const int64_t WordOffset = SignedValue >> 2;
    if (!isInt<16>(WordOffset))
      Ctx.reportError(Fixup.getLoc(), "TCore branch target out of range");
    return static_cast<uint32_t>(WordOffset) & 0xffffu;
  }
  default:
    llvm_unreachable("unknown TCore fixup kind");
  }
}

MCFixupKindInfo TCoreAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  static const MCFixupKindInfo Infos[TCore::NumTargetFixupKinds] = {
      {"fixup_tcore_abs16", 5, 16, 0},
      {"fixup_tcore_hi16", 5, 16, 0},
      {"fixup_tcore_lo16", 5, 16, 0},
      {"fixup_tcore_pcrel_16word", 5, 16, 0},
  };

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < TCore::NumTargetFixupKinds &&
         "invalid TCore fixup kind");
  return Infos[Kind - FirstTargetFixupKind];
}

void TCoreAsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                                 const MCValue &Target, uint8_t *Data,
                                 uint64_t Value, bool IsResolved) {
  if (!IsResolved)
    Asm->getWriter().recordRelocation(F, Fixup, Target, Value);

  const MCFixupKind Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return;

  const MCFixupKindInfo Info = getFixupKindInfo(Kind);
  const uint32_t FieldValue = adjustFixupValue(Fixup, Value, getContext());
  if (!FieldValue)
    return;

  uint32_t Word = support::endian::read32le(Data);
  const uint32_t Mask = ((1u << Info.TargetSize) - 1u) << Info.TargetOffset;
  Word = (Word & ~Mask) | ((FieldValue << Info.TargetOffset) & Mask);
  support::endian::write32le(Data, Word);
}

std::unique_ptr<MCObjectTargetWriter>
TCoreAsmBackend::createObjectTargetWriter() const {
  return createTCoreELFObjectWriter(OSABI);
}

bool TCoreAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                   const MCSubtargetInfo *) const {
  if (Count == 0)
    return true;

  if (Count % 4 != 0)
    return false;

  static constexpr char NopWord[4] = {0, 0, 0, 0};
  while (Count) {
    OS.write(NopWord, sizeof(NopWord));
    Count -= sizeof(NopWord);
  }
  return true;
}

MCAsmBackend *llvm::createTCoreAsmBackend(const Target &,
                                          const MCSubtargetInfo &STI,
                                          const MCRegisterInfo &,
                                          const MCTargetOptions &) {
  const uint8_t OSABI =
      MCELFObjectTargetWriter::getOSABI(STI.getTargetTriple().getOS());
  return new TCoreAsmBackend(OSABI);
}

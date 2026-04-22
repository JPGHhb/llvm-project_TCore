#ifndef LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREMCTARGETDESC_H
#define LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREMCTARGETDESC_H

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/DataTypes.h"
#include <memory>

#define GET_REGINFO_ENUM
#include "TCoreGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "TCoreGenInstrInfo.inc"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class Target;
class MCInstrInfo;
class MCTargetOptions;

MCCodeEmitter *createTCoreMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);
MCAsmBackend *createTCoreAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter> createTCoreELFObjectWriter(uint8_t OSABI);
} // namespace llvm

#endif

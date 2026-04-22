#ifndef LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREFIXUPKINDS_H
#define LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace TCore {

enum Fixups {
  fixup_tcore_abs16 = FirstTargetFixupKind,
  fixup_tcore_hi16,
  fixup_tcore_lo16,
  fixup_tcore_pcrel_16word,

  fixup_tcore_invalid,
  NumTargetFixupKinds = fixup_tcore_invalid - FirstTargetFixupKind
};

} // namespace TCore
} // namespace llvm

#endif

#ifndef LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREMCASMINFO_H
#define LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class TCoreMCAsmInfo : public MCAsmInfoELF {
public:
  explicit TCoreMCAsmInfo(const Triple &TT);
};
}

#endif

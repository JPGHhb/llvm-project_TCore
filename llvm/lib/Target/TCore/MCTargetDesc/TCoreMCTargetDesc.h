#ifndef LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREMCTARGETDESC_H
#define LLVM_LIB_TARGET_TCORE_MCTARGETDESC_TCOREMCTARGETDESC_H

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/DataTypes.h"

#define GET_REGINFO_ENUM
#include "TCoreGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "TCoreGenInstrInfo.inc"

#endif

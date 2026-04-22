#ifndef LLVM_LIB_TARGET_TCORE_TCORE_H
#define LLVM_LIB_TARGET_TCORE_TCORE_H

#include "MCTargetDesc/TCoreMCTargetDesc.h"
#include "llvm/Support/CodeGen.h"

namespace llvm {
class FunctionPass;
class TCoreTargetMachine;
class PassRegistry;

FunctionPass *createTCoreISelDag(TCoreTargetMachine &TM,
                                 CodeGenOptLevel OptLevel);
void initializeTCoreAsmPrinterPass(PassRegistry &);
}

#endif

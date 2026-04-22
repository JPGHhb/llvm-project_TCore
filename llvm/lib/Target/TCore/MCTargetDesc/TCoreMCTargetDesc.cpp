#include "MCTargetDesc/TCoreFixupKinds.h"
#include "MCTargetDesc/TCoreInstPrinter.h"
#include "MCTargetDesc/TCoreMCAsmInfo.h"
#include "MCTargetDesc/TCoreMCTargetDesc.h"
#include "TargetInfo/TCoreTargetInfo.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "TCoreGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "TCoreGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "TCoreGenRegisterInfo.inc"

static MCInstrInfo *createTCoreMCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitTCoreMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createTCoreMCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitTCoreMCRegisterInfo(X, TCore::LR);
  return X;
}

static MCSubtargetInfo *createTCoreMCSubtargetInfo(const Triple &TT,
                                                   StringRef CPU,
                                                   StringRef FS) {
  return createTCoreMCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

static MCAsmInfo *createTCoreMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new TCoreMCAsmInfo(TT);
  MAI->addInitialFrameState(MCCFIInstruction::cfiDefCfa(nullptr, TCore::SP, 0));
  return MAI;
}

static MCInstPrinter *createTCoreMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new TCoreInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTCoreTargetMC() {
  RegisterMCAsmInfoFn X(getTheTCoreTarget(), createTCoreMCAsmInfo);
  TargetRegistry::RegisterMCAsmBackend(getTheTCoreTarget(),
                                       createTCoreAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(getTheTCoreTarget(),
                                        createTCoreMCCodeEmitter);
  TargetRegistry::RegisterMCInstrInfo(getTheTCoreTarget(), createTCoreMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(getTheTCoreTarget(), createTCoreMCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(getTheTCoreTarget(),
                                          createTCoreMCSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(getTheTCoreTarget(),
                                        createTCoreMCInstPrinter);
}

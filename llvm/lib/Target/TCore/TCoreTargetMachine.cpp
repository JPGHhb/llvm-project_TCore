#include "TCoreTargetMachine.h"
#include "TCore.h"
#include "TargetInfo/TCoreTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTCoreTarget() {
  RegisterTargetMachine<TCoreTargetMachine> X(getTheTCoreTarget());
}

static std::string computeDataLayout() {
  return "e-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:32:32-"
         "f32:32:32-f64:32:32-a0:0:32-n32";
}

static Reloc::Model getEffectiveRelocModel(bool JIT,
                                           std::optional<Reloc::Model> RM) {
  if (!RM || JIT)
    return Reloc::Static;
  return *RM;
}

TCoreTargetMachine::TCoreTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(JIT, RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

TCoreTargetMachine::~TCoreTargetMachine() = default;

namespace {
class TCorePassConfig : public TargetPassConfig {
public:
  TCorePassConfig(TCoreTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  TCoreTargetMachine &getTCoreTargetMachine() const {
    return getTM<TCoreTargetMachine>();
  }

  bool addInstSelector() override {
    addPass(createTCoreISelDag(getTCoreTargetMachine(), getOptLevel()));
    return false;
  }

  void addIRPasses() override {
    addPass(createAtomicExpandLegacyPass());
    TargetPassConfig::addIRPasses();
  }
};
}

TargetPassConfig *TCoreTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TCorePassConfig(*this, PM);
}

#ifndef LLVM_LIB_TARGET_TCORE_TCORETARGETMACHINE_H
#define LLVM_LIB_TARGET_TCORE_TCORETARGETMACHINE_H

#include "TCoreSubtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Support/CodeGen.h"
#include <memory>
#include <optional>

namespace llvm {

class TCoreTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  TCoreSubtarget Subtarget;

public:
  TCoreTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                     bool JIT);
  ~TCoreTargetMachine() override;

  const TCoreSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

}

#endif

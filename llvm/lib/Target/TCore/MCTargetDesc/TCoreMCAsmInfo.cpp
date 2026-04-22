#include "TCoreMCAsmInfo.h"
#include "llvm/BinaryFormat/Dwarf.h"

using namespace llvm;

TCoreMCAsmInfo::TCoreMCAsmInfo(const Triple &TT) {
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 4;
  MinInstAlignment = 4;
  CommentString = "//";
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::None;
  UsesELFSectionDirectiveForBSS = true;
}

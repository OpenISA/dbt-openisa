#include <manager.hpp>
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using namespace dbt;

void Manager::addOIRegion(uint32_t EntryAddress, OIInstList OIRegion) {
  OIRegions[EntryAddress] = OIRegion;

  llvm::Module* M = IRE->generateRegionIR(EntryAddress, OIRegion, 0); //FIXME: Should'nt be 0!

  IRO->optimizeIRFunction(M, IROpt::OptLevel::Basic); 

  IRJIT->addModule(std::unique_ptr<llvm::Module>(M));
  NativeRegions[EntryAddress] = (intptr_t) IRJIT->findSymbol("r"+std::to_string(EntryAddress)).getAddress();
}

int32_t Manager::jumpToRegion(uint32_t EntryAddress, dbt::Machine& M) {
  uint32_t JumpTo = EntryAddress;
  while (isNativeRegionEntry(JumpTo)) {
    int32_t (*FP)(int32_t*, int32_t*) = (int32_t (*)(int32_t*, int32_t*)) NativeRegions[JumpTo];
    JumpTo = FP(M.getRegisterPtr(), M.getMemoryPtr());
  }
  return JumpTo;
}
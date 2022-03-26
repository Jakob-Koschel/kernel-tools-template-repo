#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

#include <llvm/IR/IRBuilder.h>
#include "llvm/IR/InstVisitor.h"

#include "llvm/Transforms/IPO/PassManagerBuilder.h" // RegisterStandardPasses
#include <llvm/Transforms/Utils/BasicBlockUtils.h> // SplitBlock

using namespace llvm;

cl::opt<std::string> templateHookName("template-hook",
        cl::desc("Specify the function that the hook should call."),
        cl::init("kernel_tools_hook_test"));

cl::opt<std::string> templateFunctionName("template-function",
        cl::desc("Specify the function where the hook should be inserted."),
        cl::init("__x64_sys_newuname"));

namespace {

class Template {
  Module *Mod;
  LLVMContext *Ctx;

  Function *templateHook;

  bool init(Module &M);
  bool visitor(Function &F);

public:
  bool runImpl(Module &M);
};

bool Template::init(Module &M) {
  Mod = &M;
  Ctx = &M.getContext();

  std::vector<Type*> templateHookParamTypes = {};
  Type *templateHookRetType = Type::getVoidTy(*Ctx);
  FunctionType *templateHookFuncType = FunctionType::get(
      templateHookRetType,
      templateHookParamTypes, false);
  Value *templateHookFunc  = Mod->getOrInsertFunction(
      templateHookName,
      templateHookFuncType).getCallee();
  if (templateHookFunc == NULL) {
    return false;
  }
  templateHook = cast<Function>(templateHookFunc);
  templateHook->setCallingConv(CallingConv::Fast);

  return true;
}

bool Template::visitor(Function &F) {
  bool Changed = false;
  if (F.getName().equals(templateFunctionName)) {
    for (BasicBlock &BB : F) {
      for (Instruction &I: BB) {
        IRBuilder<> builder(*Ctx);
        builder.SetInsertPoint(&I);

        std::vector<Value*> args(0);
        errs() << templateHook << "\n";
        builder.CreateCall(templateHook, args);
        Changed = true;
        break;
      }
      break;
    }
  }
  return Changed;
}

bool Template::runImpl(Module &M) {
  if (!init(M))
        return false;
  bool Changed = false;
  for (Function &F : M)
    Changed |= visitor(F);
  return true;
}

// New PM implementation
struct TemplatePass : PassInfoMixin<TemplatePass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    bool Changed = Template().runImpl(M);
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

// Legacy PM implementation
struct LegacyTemplatePass : public ModulePass {
  static char ID;
  LegacyTemplatePass() : ModulePass(ID) {}
  // Main entry point - the name conveys what unit of IR this is to be run on.
  bool runOnModule(Module &M) override {
    return Template().runImpl(M);
  }
};
} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getTemplatePassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Template", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerOptimizerLastEPCallback(
                [](llvm::ModulePassManager &PM,
                  llvm::PassBuilder::OptimizationLevel Level) {
                PM.addPass(TemplatePass());
                });
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "template") {
                    MPM.addPass(TemplatePass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getTemplatePassPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
// The address of this variable is used to uniquely identify the pass. The
// actual value doesn't matter.
char LegacyTemplatePass::ID = 0;

static RegisterPass<LegacyTemplatePass>
    X("legacy-template", "Template Pass",
      false, // This pass DOES modify the CFG => false
      false // This pass is not a pure analysis pass => false
    );

static llvm::RegisterStandardPasses RegisterTemplateLTOThinPass(
    llvm::PassManagerBuilder::EP_OptimizerLast,
    [](const llvm::PassManagerBuilder &Builder,
       llvm::legacy::PassManagerBase &PM) { PM.add(new LegacyTemplatePass()); });

static llvm::RegisterStandardPasses RegisterTemplateLTOPass(
    llvm::PassManagerBuilder::EP_FullLinkTimeOptimizationLast,
    [](const llvm::PassManagerBuilder &Builder,
       llvm::legacy::PassManagerBase &PM) { PM.add(new LegacyTemplatePass()); });

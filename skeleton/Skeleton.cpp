#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Constants.h"

using namespace llvm;

namespace {

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        errs() << "\n";
        errs() << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
        errs() << "║                           🔍 LLVM MODULE ANALYSIS                            ║\n";
        errs() << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
        errs() << "📁 Module: " << M.getName() << "\n";
        errs() << "══════════════════════════════════════════════════════════════════════════════\n\n";

        for (Function &F : M) {
            // Skip function declarations (external functions)
            if (F.isDeclaration()) {
                errs() << "📋 External Function Declaration: " << F.getName() << "()\n";
                errs() << "   ↳ Return Type: " << *F.getReturnType() << "\n";
                errs() << "   ↳ Parameters: " << F.arg_size() << "\n";
                if (F.arg_size() > 0) {
                    for (auto &arg : F.args()) {
                        errs() << "     • " << (arg.hasName() ? arg.getName() : "unnamed") 
                               << " : " << *arg.getType() << "\n";
                    }
                }
                errs() << "\n";
                continue;
            }

            errs() << "🔧 Function Definition: " << F.getName() << "()\n";
            errs() << "   ↳ Return Type: " << *F.getReturnType() << "\n";
            errs() << "   ↳ Parameters: " << F.arg_size() << "\n";
            errs() << "   ↳ Basic Blocks: " << F.size() << "\n";
            
            if (F.arg_size() > 0) {
                errs() << "   ↳ Function Arguments:\n";
                for (auto &arg : F.args()) {
                    errs() << "     • " << (arg.hasName() ? arg.getName() : "unnamed") 
                           << " : " << *arg.getType() << "\n";
                }
            }
            errs() << "\n";

            unsigned bbCount = 0;
            for (BasicBlock &BB : F) {
                bbCount++;
                errs() << "   ┌─ Basic Block #" << bbCount << ": " 
                       << (BB.hasName() ? BB.getName() : "unnamed") << "\n";
                errs() << "   │  Instructions: " << BB.size() << "\n";
                errs() << "   │\n";

                unsigned instCount = 0;
                for (Instruction &I : BB) {
                    instCount++;
                    errs() << "   │  [" << instCount << "] " << I << "\n";

                    // Check for different instruction types with detailed analysis
                    if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
                        errs() << "   │      🔧 Binary Operation: " << binOp->getOpcodeName() << "\n";
                        errs() << "   │         Operand 1: " << *binOp->getOperand(0) << "\n";
                        errs() << "   │         Operand 2: " << *binOp->getOperand(1) << "\n";
                        
                    } else if (auto *alloca = dyn_cast<AllocaInst>(&I)) {
                        errs() << "   │      📦 Stack Allocation (alloca)\n";
                        errs() << "   │         Type: " << *alloca->getAllocatedType() << "\n";
                        errs() << "   │         Size: " << alloca->getAllocationSize(M.getDataLayout()) << " bytes\n";
                        errs() << "   │         Alignment: " << alloca->getAlign().value() << " bytes\n";
                        
                    } else if (auto *load = dyn_cast<LoadInst>(&I)) {
                        errs() << "   │      📥 Load from Memory\n";
                        errs() << "   │         Source: " << *load->getPointerOperand() << "\n";
                        errs() << "   │         Type: " << *load->getType() << "\n";
                        errs() << "   │         Alignment: " << load->getAlign().value() << " bytes\n";
                        
                    } else if (auto *store = dyn_cast<StoreInst>(&I)) {
                        errs() << "   │      📤 Store to Memory\n";
                        errs() << "   │         Value: " << *store->getValueOperand() << "\n";
                        errs() << "   │         Destination: " << *store->getPointerOperand() << "\n";
                        errs() << "   │         Alignment: " << store->getAlign().value() << " bytes\n";
                        
                    } else if (auto *call = dyn_cast<CallInst>(&I)) {
                        if (call->getCalledFunction()) {
                            errs() << "   │      📞 Function Call: " << call->getCalledFunction()->getName() << "()\n";
                            errs() << "   │         Arguments: " << call->arg_size() << "\n";
                            
                            unsigned argNum = 0;
                            for (auto &arg : call->args()) {
                                argNum++;
                                errs() << "   │         Arg " << argNum << ": " << *arg << "\n";
                            }
                            
                            // Show function signature
                            errs() << "   │         Target Function Signature:\n";
                            for (auto &param : call->getCalledFunction()->args()) {
                                errs() << "   │           • " << (param.hasName() ? param.getName() : "unnamed") 
                                       << " : " << *param.getType() << "\n";
                            }
                        } else {
                            errs() << "   │      📞 Indirect Function Call\n";
                            errs() << "   │         Target: " << *call->getCalledOperand() << "\n";
                        }
                        
                    } else if (auto *br = dyn_cast<BranchInst>(&I)) {
                        if (br->isConditional()) {
                            errs() << "   │      🔀 Conditional Branch\n";
                            errs() << "   │         Condition: " << *br->getCondition() << "\n";
                            errs() << "   │         True Block: " << br->getSuccessor(0)->getName() << "\n";
                            errs() << "   │         False Block: " << br->getSuccessor(1)->getName() << "\n";
                        } else {
                            errs() << "   │      ➡️  Unconditional Branch\n";
                            errs() << "   │         Target: " << br->getSuccessor(0)->getName() << "\n";
                        }
                        
                    } else if (auto *ret = dyn_cast<ReturnInst>(&I)) {
                        if (ret->getReturnValue()) {
                            Value *retVal = ret->getReturnValue();
                            errs() << "   │      🔙 Return Statement\n";
                            errs() << "   │         Type: " << *retVal->getType() << "\n";
                            
                            if (retVal->hasName()) {
                                errs() << "   │         Value: " << retVal->getName() << "\n";
                            } else {
                                errs() << "   │         Value: (unnamed temporary)\n";
                                if (auto *inst = dyn_cast<Instruction>(retVal)) {
                                    errs() << "   │         Source: " << *inst << "\n";
                                } else if (auto *constant = dyn_cast<ConstantInt>(retVal)) {
                                    errs() << "   │         Constant: " << constant->getSExtValue() << "\n";
                                }
                            }
                        } else {
                            errs() << "   │      🔙 Return Statement (void)\n";
                        }
                        
                    } else if (auto *cmp = dyn_cast<CmpInst>(&I)) {
                        errs() << "   │      ⚖️  Comparison Instruction\n";
                        if (auto *icmp = dyn_cast<ICmpInst>(&I)) {
                            errs() << "   │         Type: Integer Comparison\n";
                            errs() << "   │         Predicate: ";
                            switch (icmp->getPredicate()) {
                                case CmpInst::ICMP_EQ:  errs() << "Equal (==)"; break;
                                case CmpInst::ICMP_NE:  errs() << "Not Equal (!=)"; break;
                                case CmpInst::ICMP_SGT: errs() << "Signed Greater Than (>)"; break;
                                case CmpInst::ICMP_SGE: errs() << "Signed Greater or Equal (>=)"; break;
                                case CmpInst::ICMP_SLT: errs() << "Signed Less Than (<)"; break;
                                case CmpInst::ICMP_SLE: errs() << "Signed Less or Equal (<=)"; break;
                                default: errs() << "Other"; break;
                            }
                            errs() << "\n";
                        }
                        errs() << "   │         Left Operand: " << *cmp->getOperand(0) << "\n";
                        errs() << "   │         Right Operand: " << *cmp->getOperand(1) << "\n";
                        
                    } else if (auto *cast = dyn_cast<CastInst>(&I)) {
                        errs() << "   │      🔄 Cast Operation: " << cast->getOpcodeName() << "\n";
                        errs() << "   │         From: " << *cast->getSrcTy() << "\n";
                        errs() << "   │         To: " << *cast->getDestTy() << "\n";
                        errs() << "   │         Source: " << *cast->getOperand(0) << "\n";
                        
                    } else if (auto *op = dyn_cast<Operator>(&I)) {
                        errs() << "   │      ⚙️  Other Operator: " << I.getOpcodeName() << "\n";
                        errs() << "   │         Operands: " << I.getNumOperands() << "\n";
                        for (unsigned i = 0; i < I.getNumOperands(); ++i) {
                            errs() << "   │         Op[" << i << "]: " << *I.getOperand(i) << "\n";
                        }
                        
                    } else {
                        errs() << "   │      ❓ Unknown Instruction Type\n";
                        errs() << "   │         Opcode: " << I.getOpcodeName() << "\n";
                    }
                    errs() << "   │\n";
                }
                
                // Check if this is not the last basic block
                if (&BB != &F.back()) {
                    errs() << "   ├─────────────────────────────────────────────────────\n";
                } else {
                    errs() << "   └─────────────────────────────────────────────────────\n";
                }
            }
            
            errs() << "\n══════════════════════════════════════════════════════════════════════════════\n\n";
        }

        errs() << "✅ Analysis Complete!\n";
        errs() << "═══════════════════════════════════════════════════════════════════════════════\n\n";
        
        return PreservedAnalyses::all();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Enhanced Skeleton Pass",
        .PluginVersion = "v2.0",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}

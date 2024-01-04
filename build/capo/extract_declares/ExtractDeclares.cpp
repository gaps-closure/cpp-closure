#include <iostream>
#include <fstream>
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
struct ExtractDeclares : public PassInfoMixin<ExtractDeclares> {
    PreservedAnalyses run(Module &module, ModuleAnalysisManager &MAM) {

        unsigned int ll_fn_instruction_num;
        StringRef ll_fn_name;

        StringRef src_file_name;
        StringRef src_variable_name;
        unsigned int src_location_line;
        unsigned int src_location_col;

        unsigned int param_pos;

        // https://llvm.org/docs/SourceLevelDebugging.html#llvm-dbg-declare
        Metadata * declare_arg_1;  // The first argument is metadata holding the address of variable
        Metadata * declare_arg_2;  // The second argument is a local variable containing a description of the variable
        Metadata * declare_arg_3;  // The third argument is a complex expression.
        User::op_iterator declare_arg_it;

        std::ofstream csv_file;
        csv_file.open("declares.csv");

        // Global declarations
        for (auto &global_var : module.getGlobalList()) {
            src_variable_name = global_var.getName();

            SmallVector<DIGlobalVariableExpression *, 1> debug_info;
            global_var.getDebugInfo(debug_info);

            if (debug_info.size() != 1) {
                errs() << "WARNING: Unexpected debug information for global, just getting the first";
            }
            for (auto *GVE : debug_info) {
                DIVariable * variable_debug_info = GVE->getVariable();
                src_file_name = variable_debug_info->getFilename();
                src_location_line = variable_debug_info->getLine();
                break;
            }

            csv_file 
                << src_variable_name.str() << "::, "
                << src_file_name.str() << ":"
                << src_location_line << ":, "
                << src_variable_name.str() << "\n";
        }

        // Local declarations via @llvm.dbg.declare
        for (auto &caller_function : module) {

            ll_fn_instruction_num = 0;
            for (auto &basic_block : caller_function.getBasicBlockList()) {
                for (auto &instruction : basic_block.getInstList()) {
                    ll_fn_instruction_num++;
                    if (DbgDeclareInst *declare_call_inst = dyn_cast<DbgDeclareInst>(&instruction)) {

                        ll_fn_name = caller_function.getName();
                        param_pos = 0;
                        src_variable_name = "";

                        src_file_name = declare_call_inst->getDebugLoc()->getFilename();
                        src_location_line = declare_call_inst->getDebugLoc()->getLine();
                        src_location_col = declare_call_inst->getDebugLoc()->getColumn();

                        declare_arg_it = declare_call_inst->arg_begin();
                        declare_arg_1 = dyn_cast<MetadataAsValue>(declare_arg_it->get())->getMetadata();
                        declare_arg_it++;
                        declare_arg_2 = dyn_cast<MetadataAsValue>(declare_arg_it->get())->getMetadata();
                        declare_arg_it++;
                        declare_arg_3 = dyn_cast<MetadataAsValue>(declare_arg_it->get())->getMetadata();
                            
                        if (auto *address_of_variable = dyn_cast<ValueAsMetadata>(declare_arg_1)) {
                            if (AllocaInst *referenced_instruction = dyn_cast<AllocaInst>(address_of_variable->getValue())) {
                                // unused currently
                            } else {
                                errs() << "ERROR: First argument not an allocation instruction reference\n";
                            }
                        } else {
                            errs() << "ERROR: First argument in unexpected form\n";
                        }

                        if (auto *description_of_variable = dyn_cast<DILocalVariable>(declare_arg_2)) {
                            src_variable_name = description_of_variable->getName();
                            param_pos = description_of_variable->getArg();
                        } else {
                            errs() << "ERROR: Second argument in unexpected form\n";
                        }
                            
                        if (auto *expression = dyn_cast<DIExpression>(declare_arg_3)) {
                            // unused currently
                        } else {
                            errs() << "ERROR: Third argument in unexpected form\n";
                        }

                        if (param_pos) {
                            csv_file 
                                << ll_fn_name.str() << "::"
                                << param_pos << ", ";
                        } else {
                            csv_file 
                                << ll_fn_name.str() << ":"
                                << ll_fn_instruction_num << ":, ";
                        }
                        
                        csv_file
                            << src_file_name.str() << ":"
                            << src_location_line << ":"
                            << src_location_col << ", "
                            << src_variable_name.str() << "\n";

                    }
                }
            }
        }
        
        csv_file.close();
        return PreservedAnalyses::all();
    }
};
}  // end anonymous namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "ExtractDeclares",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "declare-csv") {
                        MPM.addPass(ExtractDeclares());
                        return true;
                    }
                    return false;
                });
        }};
}
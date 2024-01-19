#include <iostream>
#include <fstream>
#include <unordered_map>
#include <filesystem>

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;

namespace {

unsigned long long int get_file_offset(std::string src_file_path, unsigned int line, unsigned int column) {

    char ch;
    unsigned int line_count = 1;
    unsigned int column_count = 1;
    unsigned long long int file_offset = 0;

    if (line < 1 || column < 1) {
        return 0;
    }

    FILE * file_stream = fopen(src_file_path.c_str(), "r");

    while(fread(&ch, 1, 1, file_stream)) {
        file_offset++;
        if (ch == EOF) break;
        if (ch == '\n') {
            line_count ++;
            column_count = 1;
        }

        if (line_count == line && column_count == column) {
            fclose(file_stream);
            return file_offset;
        }

        column_count++;
    }

    fclose(file_stream);
    return ~0;
}


struct ExtractDeclares : public PassInfoMixin<ExtractDeclares> {
    PreservedAnalyses run(Module &module, ModuleAnalysisManager &MAM) {

        unsigned int ll_fn_instruction_num;
        unsigned int referenced_fn_instruction_num;
        StringRef ll_fn_name;

        std::filesystem::path src_file_dir;
        std::filesystem::path src_file_name;
        StringRef src_variable_name;
        unsigned int src_location_line;
        unsigned int src_location_col;
        unsigned long long int src_location_file_offset;

        unsigned int param_pos;
        std::unordered_map<void *, unsigned int> ll_fn_instruction_map;

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

                src_file_dir = variable_debug_info->getDirectory().str();
                src_file_name = variable_debug_info->getFilename().str();
                if (!src_file_name.has_root_path()) {
                    src_file_name = src_file_dir / src_file_name;
                }

                src_location_line = variable_debug_info->getLine();
                src_location_file_offset = get_file_offset(src_file_name, src_location_line, 1);
                break;
            }

            csv_file 
                << src_variable_name.str() << ",,,"
                << src_file_name << ","
                << src_location_file_offset << ",,"
                << src_variable_name.str() << "\n";
        }

        // Local declarations via @llvm.dbg.declare
        for (auto &caller_function : module) {

            ll_fn_instruction_num = 0;
            for (auto &basic_block : caller_function.getBasicBlockList()) {
                for (auto &instruction : basic_block.getInstList()) {

                    referenced_fn_instruction_num = ~0;
                    ll_fn_instruction_map[&instruction] = ll_fn_instruction_num;

                    if (DbgDeclareInst *declare_call_inst = dyn_cast<DbgDeclareInst>(&instruction)) {

                        ll_fn_name = caller_function.getName();
                        param_pos = 0;
                        src_variable_name = "";

                        src_file_dir = declare_call_inst->getDebugLoc()->getDirectory().str();
                        src_file_name = declare_call_inst->getDebugLoc()->getFilename().str();
                        if (!src_file_name.has_root_path()) {
                            src_file_name = src_file_dir / src_file_name;
                        }

                        src_location_line = declare_call_inst->getDebugLoc()->getLine();
                        src_location_col = declare_call_inst->getDebugLoc()->getColumn();
                        src_location_file_offset = get_file_offset(src_file_name, src_location_line, src_location_col);

                        declare_arg_it = declare_call_inst->arg_begin();
                        declare_arg_1 = dyn_cast<MetadataAsValue>(declare_arg_it->get())->getMetadata();
                        declare_arg_it++;
                        declare_arg_2 = dyn_cast<MetadataAsValue>(declare_arg_it->get())->getMetadata();
                        declare_arg_it++;
                        declare_arg_3 = dyn_cast<MetadataAsValue>(declare_arg_it->get())->getMetadata();
                            
                        if (auto *address_of_variable = dyn_cast<ValueAsMetadata>(declare_arg_1)) {
                            if (AllocaInst *referenced_instruction = dyn_cast<AllocaInst>(address_of_variable->getValue())) {
                                referenced_fn_instruction_num = ll_fn_instruction_map[address_of_variable->getValue()];
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
                                << ll_fn_name.str() << ",,"
                                << param_pos << ",";
                        } else {
                            csv_file 
                                << ll_fn_name.str() << ","
                                << referenced_fn_instruction_num << ",,";
                        }
                        
                        csv_file
                            << src_file_name << ","
                            << src_location_file_offset << ","
                            << src_variable_name.str() << "\n";

                    }
                    ll_fn_instruction_num++;
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
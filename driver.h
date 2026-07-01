#pragma once
#include <string>
#include <vector>

namespace cminus {

struct CompilerOptions {
    std::string              input_file;
    std::string              output_file   = "a.out";
    std::string              asm_file;
    bool                     dump_ast      = false;
    bool                     dump_tokens   = false;
    bool                     asm_only      = false;
    bool                     obj_only      = false;
    bool                     verbose       = false;
    std::vector<std::string> include_paths;
    std::vector<std::string> linker_flags;
    std::string              target        = "linux-x86_64";
};

bool parse_args(int argc, char** argv, CompilerOptions& opts);
int compile(const CompilerOptions& opts);
void print_usage(const char* argv0);

} // namespace cminus

#include "driver.h"
#include "lexer.h"
#include "parser.h"
#include "typechecker.h"
#include "codegen.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <cstring>

namespace cminus {

bool parse_args(int argc, char** argv, CompilerOptions& opts) {
    if (argc < 2) {
        print_usage(argv[0]);
        return false;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "error: -o requires an argument\n");
                    return false;
                }
                opts.output_file = argv[++i];
            } else if (strcmp(argv[i], "-S") == 0) {
                opts.asm_only = true;
            } else if (strcmp(argv[i], "-c") == 0) {
                opts.obj_only = true;
            } else if (strcmp(argv[i], "-dump-ast") == 0) {
                opts.dump_ast = true;
            } else if (strcmp(argv[i], "-dump-tokens") == 0) {
                opts.dump_tokens = true;
            } else if (strcmp(argv[i], "-v") == 0) {
                opts.verbose = true;
            } else if (strcmp(argv[i], "-I") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "error: -I requires an argument\n");
                    return false;
                }
                opts.include_paths.push_back(argv[++i]);
            } else {
                fprintf(stderr, "error: unknown option: %s\n", argv[i]);
                return false;
            }
        } else {
            opts.input_file = argv[i];
        }
    }

    if (opts.input_file.empty()) {
        fprintf(stderr, "error: no input file\n");
        return false;
    }

    return true;
}

int compile(const CompilerOptions& opts) {
    if (opts.verbose) {
        printf("C- Compiler\n");
        printf("Input:  %s\n", opts.input_file.c_str());
        printf("Output: %s\n", opts.output_file.c_str());
    }

    std::ifstream input(opts.input_file);
    if (!input) {
        fprintf(stderr, "error: cannot open input file: %s\n", opts.input_file.c_str());
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(input)),
                       std::istreambuf_iterator<char>());

    Lexer lexer(opts.input_file.c_str(), source);
    auto tokens = lexer.tokenize();

    if (opts.dump_tokens) {
        printf("Tokens:\n");
        for (const auto& tok : tokens) {
            printf("  %s: %s\n", tok.kind_name(), tok.text.c_str());
        }
    }

    if (lexer.has_errors()) {
        fprintf(stderr, "error: lexical analysis failed\n");
        return 1;
    }

    Parser parser(tokens, opts.input_file.c_str());
    auto module = parser.parse_module("main");

    if (parser.has_errors()) {
        fprintf(stderr, "error: parsing failed\n");
        return 1;
    }

    if (opts.dump_ast) {
        printf("AST: %lu declarations\n", module->decls.size());
    }

    TypeChecker tc;
    if (!tc.check(*module)) {
        fprintf(stderr, "error: type checking failed\n");
        return 1;
    }

    Codegen codegen(*module, tc);
    std::string asm_code = codegen.emit_asm();

    if (opts.asm_only || opts.obj_only) {
        std::string asm_file = opts.asm_only ? opts.output_file : opts.output_file + ".s";
        std::ofstream out(asm_file);
        out << asm_code;
        out.close();

        if (opts.verbose) {
            printf("Generated assembly: %s\n", asm_file.c_str());
        }

        if (opts.asm_only) {
            return 0;
        }
    }

    if (opts.verbose) {
        printf("Compilation successful\n");
    }

    return 0;
}

void print_usage(const char* argv0) {
    printf("Usage: %s [options] <input.cm>\n", argv0);
    printf("Options:\n");
    printf("  -o <file>    Output file\n");
    printf("  -S           Emit assembly only\n");
    printf("  -c           Emit object file\n");
    printf("  -dump-ast    Dump abstract syntax tree\n");
    printf("  -dump-tokens Dump tokens\n");
    printf("  -v           Verbose output\n");
    printf("  -I <dir>     Add include path\n");
}

} // namespace cminus

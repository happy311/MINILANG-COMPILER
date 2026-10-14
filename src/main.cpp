// main.cpp - Drives the whole pipeline:
//   source -> tokens -> AST -> IR -> (optimized IR) -> bytecode -> run

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "codegen.h"
#include "ir.h"
#include "lexer.h"
#include "optimizer.h"
#include "parser.h"
#include "vm.h"

static void printUsage() {
    std::cout << "Usage: minicc <file.ml> [options]\n"
              << "Options:\n"
              << "  --ir       print the IR (before and after optimization)\n"
              << "  --asm      print the generated assembly\n"
              << "  --no-opt   skip the optimizer\n"
              << "  --no-run   compile only, do not run the program\n";
}

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("cannot open file '" + path + "'");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string path;
    bool showIR = false;
    bool showAsm = false;
    bool runOptimizer = true;
    bool runProgramFlag = true;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--ir") {
            showIR = true;
        } else if (arg == "--asm") {
            showAsm = true;
        } else if (arg == "--no-opt") {
            runOptimizer = false;
        } else if (arg == "--no-run") {
            runProgramFlag = false;
        } else {
            path = arg;
        }
    }

    try {
        // 1. Lexing
        Lexer lexer(readFile(path));
        std::vector<Token> tokens = lexer.tokenize();

        // 2. Parsing
        Parser parser(std::move(tokens));
        std::vector<StmtPtr> ast = parser.parseProgram();

        // 3. IR generation
        IRProgram ir = generateIR(ast);
        if (showIR) {
            std::cout << "=== IR (before optimization) ===\n";
            printIR(ir, std::cout);
        }

        // 4. Optimization
        if (runOptimizer) {
            optimize(ir);
            if (showIR) {
                std::cout << "\n=== IR (after optimization) ===\n";
                printIR(ir, std::cout);
            }
        }

        // 5. Code generation
        VMProgram program = generateCode(ir);
        if (showAsm) {
            std::cout << "\n=== Assembly ===\n";
            printAssembly(program, std::cout);
        }

        // 6. Execution
        if (runProgramFlag) {
            if (showIR || showAsm) {
                std::cout << "\n=== Output ===\n";
            }
            runProgram(program, std::cout);
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

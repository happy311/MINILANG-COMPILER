// ir.h - Stage 3: Intermediate Representation (three-address code).
//
// The AST is flattened into simple instructions that each do one thing:
//     %t1 = a * b
//     x   = %t1
//     ifz %t2 goto L1
// Names starting with '%' are compiler temporaries; everything else is a
// user variable.
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "ast.h"

// An operand is either an immediate number or a named variable/temporary.
struct Operand {
    bool isImmediate = false;
    Value imm = 0;
    std::string name;

    static Operand immediate(Value value);
    static Operand variable(const std::string& name);
};

enum class InstrKind {
    Copy,        // dst = a
    Binary,      // dst = a op b
    Label,       // label:
    Jump,        // goto label
    JumpIfZero,  // ifz a goto label
    Print        // print a
};

struct Instr {
    InstrKind kind = InstrKind::Copy;
    BinaryOp op = BinaryOp::Add;  // Binary only
    std::string dst;              // Copy / Binary: destination name
    Operand a;                    // first operand
    Operand b;                    // second operand (Binary only)
    std::string label;            // Label / Jump / JumpIfZero
};

using IRProgram = std::vector<Instr>;

// True for compiler-generated temporaries such as "%t3".
bool isTemp(const std::string& name);

// Lowers the AST to IR. Also checks that variables are declared.
// Throws std::runtime_error on semantic errors.
IRProgram generateIR(const std::vector<StmtPtr>& program);

// Pretty-printing helpers.
std::string instrToString(const Instr& instr);
void printIR(const IRProgram& program, std::ostream& out);

// codegen.h - Stage 5: IR -> stack-machine bytecode.
//
// Target machine: a stack with variable slots.
//   PUSH n      push constant n
//   LOAD s      push the value of variable slot s
//   STORE s     pop a value into variable slot s
//   ADD, SUB... pop two values, push the result
//   JMP a       jump to instruction address a
//   JZ a        pop a value, jump to a if it is zero
//   PRINT       pop a value and print it
//   HALT        stop
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "ast.h"
#include "ir.h"

enum class OpCode { Push, Load, Store, Binary, Jump, JumpIfZero, Print, Halt };

struct Bytecode {
    OpCode opcode = OpCode::Halt;
    Value operand = 0;                // constant, slot number or jump address
    BinaryOp binOp = BinaryOp::Add;   // only for OpCode::Binary
};

struct VMProgram {
    std::vector<Bytecode> code;
    std::vector<std::string> varNames;  // varNames[slot] = variable name
};

VMProgram generateCode(const IRProgram& ir);

// Prints a human-readable assembly listing.
void printAssembly(const VMProgram& program, std::ostream& out);

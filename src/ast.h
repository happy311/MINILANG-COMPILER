// ast.h - Abstract Syntax Tree (AST) definitions shared by the whole compiler.
#pragma once

#include <memory>
#include <string>
#include <vector>

// Every MiniLang value is a 64-bit signed integer.
using Value = long long;

// Binary operators supported by the language.
enum class BinaryOp { Add, Sub, Mul, Div, Lt, Gt, Le, Ge, Eq, Ne };

// Printable symbol of an operator, e.g. "+".
const char* opSymbol(BinaryOp op);

// Computes "a op b". The caller must make sure b != 0 for Div.
// Comparisons return 1 (true) or 0 (false).
Value applyOp(BinaryOp op, Value a, Value b);

struct Expr;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// An expression: a number, a variable, or "lhs op rhs".
struct Expr {
    enum class Kind { Number, Variable, Binary };

    Kind kind = Kind::Number;
    Value number = 0;             // used when kind == Number
    std::string name;             // used when kind == Variable
    BinaryOp op = BinaryOp::Add;  // used when kind == Binary
    ExprPtr lhs;                  // used when kind == Binary
    ExprPtr rhs;                  // used when kind == Binary
    int line = 0;                 // source line, for error messages
};

// A statement.
struct Stmt {
    enum class Kind { Let, Assign, Print, If, While };

    Kind kind = Kind::Print;
    std::string name;               // Let / Assign: the variable name
    ExprPtr expr;                   // value, or the condition of If / While
    std::vector<StmtPtr> body;      // If-then block, or While body
    std::vector<StmtPtr> elseBody;  // If-else block (may be empty)
    bool hasElse = false;
    int line = 0;
};

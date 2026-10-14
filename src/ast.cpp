#include "ast.h"

const char* opSymbol(BinaryOp op) {
    switch (op) {
        case BinaryOp::Add: return "+";
        case BinaryOp::Sub: return "-";
        case BinaryOp::Mul: return "*";
        case BinaryOp::Div: return "/";
        case BinaryOp::Lt:  return "<";
        case BinaryOp::Gt:  return ">";
        case BinaryOp::Le:  return "<=";
        case BinaryOp::Ge:  return ">=";
        case BinaryOp::Eq:  return "==";
        case BinaryOp::Ne:  return "!=";
    }
    return "?";
}

Value applyOp(BinaryOp op, Value a, Value b) {
    switch (op) {
        case BinaryOp::Add: return a + b;
        case BinaryOp::Sub: return a - b;
        case BinaryOp::Mul: return a * b;
        case BinaryOp::Div: return a / b;
        case BinaryOp::Lt:  return a < b;
        case BinaryOp::Gt:  return a > b;
        case BinaryOp::Le:  return a <= b;
        case BinaryOp::Ge:  return a >= b;
        case BinaryOp::Eq:  return a == b;
        case BinaryOp::Ne:  return a != b;
    }
    return 0;
}

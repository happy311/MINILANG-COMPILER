#include "ir.h"

#include <ostream>
#include <set>
#include <stdexcept>

// ---------- Operand ----------

Operand Operand::immediate(Value value) {
    Operand operand;
    operand.isImmediate = true;
    operand.imm = value;
    return operand;
}

Operand Operand::variable(const std::string& name) {
    Operand operand;
    operand.name = name;
    return operand;
}

bool isTemp(const std::string& name) {
    return !name.empty() && name[0] == '%';
}

// ---------- AST -> IR ----------

namespace {

class IRGenerator {
public:
    IRProgram run(const std::vector<StmtPtr>& program) {
        for (const StmtPtr& stmt : program) {
            genStmt(*stmt);
        }
        return std::move(out_);
    }

private:
    // --- naming helpers ---
    std::string newTemp() { return "%t" + std::to_string(++tempCount_); }
    std::string newLabel() { return "L" + std::to_string(++labelCount_); }

    [[noreturn]] void error(int line, const std::string& message) const {
        throw std::runtime_error("line " + std::to_string(line) + ": " + message);
    }

    // --- emit helpers ---
    void emitCopy(const std::string& dst, const Operand& src) {
        Instr instr;
        instr.kind = InstrKind::Copy;
        instr.dst = dst;
        instr.a = src;
        out_.push_back(instr);
    }

    void emitBinary(BinaryOp op, const std::string& dst, const Operand& a, const Operand& b) {
        Instr instr;
        instr.kind = InstrKind::Binary;
        instr.op = op;
        instr.dst = dst;
        instr.a = a;
        instr.b = b;
        out_.push_back(instr);
    }

    void emitLabel(const std::string& label) {
        Instr instr;
        instr.kind = InstrKind::Label;
        instr.label = label;
        out_.push_back(instr);
    }

    void emitJump(const std::string& label) {
        Instr instr;
        instr.kind = InstrKind::Jump;
        instr.label = label;
        out_.push_back(instr);
    }

    void emitJumpIfZero(const Operand& cond, const std::string& label) {
        Instr instr;
        instr.kind = InstrKind::JumpIfZero;
        instr.a = cond;
        instr.label = label;
        out_.push_back(instr);
    }

    void emitPrint(const Operand& value) {
        Instr instr;
        instr.kind = InstrKind::Print;
        instr.a = value;
        out_.push_back(instr);
    }

    // --- expressions: returns the operand holding the result ---
    Operand genExpr(const Expr& expr) {
        switch (expr.kind) {
            case Expr::Kind::Number:
                return Operand::immediate(expr.number);

            case Expr::Kind::Variable:
                if (declared_.count(expr.name) == 0) {
                    error(expr.line, "use of undeclared variable '" + expr.name + "'");
                }
                return Operand::variable(expr.name);

            case Expr::Kind::Binary: {
                Operand left = genExpr(*expr.lhs);
                Operand right = genExpr(*expr.rhs);
                std::string temp = newTemp();
                emitBinary(expr.op, temp, left, right);
                return Operand::variable(temp);
            }
        }
        return Operand::immediate(0);
    }

    // --- statements ---
    void genBlock(const std::vector<StmtPtr>& block) {
        for (const StmtPtr& stmt : block) {
            genStmt(*stmt);
        }
    }

    void genStmt(const Stmt& stmt) {
        switch (stmt.kind) {
            case Stmt::Kind::Let: {
                Operand value = genExpr(*stmt.expr);
                if (declared_.count(stmt.name) != 0) {
                    error(stmt.line, "variable '" + stmt.name + "' is already declared");
                }
                declared_.insert(stmt.name);
                emitCopy(stmt.name, value);
                break;
            }

            case Stmt::Kind::Assign: {
                if (declared_.count(stmt.name) == 0) {
                    error(stmt.line, "assignment to undeclared variable '" + stmt.name + "'");
                }
                Operand value = genExpr(*stmt.expr);
                emitCopy(stmt.name, value);
                break;
            }

            case Stmt::Kind::Print: {
                Operand value = genExpr(*stmt.expr);
                emitPrint(value);
                break;
            }

            case Stmt::Kind::If: {
                std::string elseLabel = newLabel();
                std::string endLabel = newLabel();
                Operand cond = genExpr(*stmt.expr);

                // If the condition is false, skip the "then" block.
                emitJumpIfZero(cond, stmt.hasElse ? elseLabel : endLabel);
                genBlock(stmt.body);

                if (stmt.hasElse) {
                    emitJump(endLabel);
                    emitLabel(elseLabel);
                    genBlock(stmt.elseBody);
                }
                emitLabel(endLabel);
                break;
            }

            case Stmt::Kind::While: {
                std::string startLabel = newLabel();
                std::string endLabel = newLabel();

                emitLabel(startLabel);
                Operand cond = genExpr(*stmt.expr);
                emitJumpIfZero(cond, endLabel);
                genBlock(stmt.body);
                emitJump(startLabel);
                emitLabel(endLabel);
                break;
            }
        }
    }

    IRProgram out_;
    std::set<std::string> declared_;
    int tempCount_ = 0;
    int labelCount_ = 0;
};

std::string operandToString(const Operand& operand) {
    if (operand.isImmediate) {
        return std::to_string(operand.imm);
    }
    return operand.name;
}

}  // namespace

IRProgram generateIR(const std::vector<StmtPtr>& program) {
    IRGenerator generator;
    return generator.run(program);
}

std::string instrToString(const Instr& instr) {
    switch (instr.kind) {
        case InstrKind::Copy:
            return "    " + instr.dst + " = " + operandToString(instr.a);
        case InstrKind::Binary:
            return "    " + instr.dst + " = " + operandToString(instr.a) + " " +
                   opSymbol(instr.op) + " " + operandToString(instr.b);
        case InstrKind::Label:
            return instr.label + ":";
        case InstrKind::Jump:
            return "    goto " + instr.label;
        case InstrKind::JumpIfZero:
            return "    ifz " + operandToString(instr.a) + " goto " + instr.label;
        case InstrKind::Print:
            return "    print " + operandToString(instr.a);
    }
    return "";
}

void printIR(const IRProgram& program, std::ostream& out) {
    for (const Instr& instr : program) {
        out << instrToString(instr) << "\n";
    }
}

#include "vm.h"

#include <ostream>
#include <stdexcept>
#include <vector>

void runProgram(const VMProgram& program, std::ostream& out) {
    const long long maxSteps = 50000000;  // protects against infinite loops

    std::vector<Value> stack;
    std::vector<Value> vars(program.varNames.size(), 0);
    size_t pc = 0;  // program counter
    long long steps = 0;

    auto pop = [&]() {
        Value top = stack.back();
        stack.pop_back();
        return top;
    };

    while (true) {
        if (++steps > maxSteps) {
            throw std::runtime_error("program ran too long (infinite loop?)");
        }

        const Bytecode& ins = program.code[pc];
        pc++;

        switch (ins.opcode) {
            case OpCode::Push:
                stack.push_back(ins.operand);
                break;

            case OpCode::Load:
                stack.push_back(vars[ins.operand]);
                break;

            case OpCode::Store:
                vars[ins.operand] = pop();
                break;

            case OpCode::Binary: {
                Value right = pop();
                Value left = pop();
                if (ins.binOp == BinaryOp::Div && right == 0) {
                    throw std::runtime_error("division by zero");
                }
                stack.push_back(applyOp(ins.binOp, left, right));
                break;
            }

            case OpCode::Jump:
                pc = static_cast<size_t>(ins.operand);
                break;

            case OpCode::JumpIfZero:
                if (pop() == 0) {
                    pc = static_cast<size_t>(ins.operand);
                }
                break;

            case OpCode::Print:
                out << pop() << "\n";
                break;

            case OpCode::Halt:
                return;
        }
    }
}

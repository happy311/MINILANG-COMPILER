#include "codegen.h"

#include <iomanip>
#include <ostream>
#include <unordered_map>

namespace {

class CodeGenerator {
public:
    VMProgram run(const IRProgram& ir) {
        for (const Instr& instr : ir) {
            genInstr(instr);
        }
        emit(OpCode::Halt);
        patchJumps();
        return std::move(program_);
    }

private:
    // Appends one bytecode instruction and returns its address.
    size_t emit(OpCode opcode, Value operand = 0, BinaryOp binOp = BinaryOp::Add) {
        Bytecode bytecode;
        bytecode.opcode = opcode;
        bytecode.operand = operand;
        bytecode.binOp = binOp;
        program_.code.push_back(bytecode);
        return program_.code.size() - 1;
    }

    // Every variable/temporary gets a numbered slot the first time it is seen.
    Value slotFor(const std::string& name) {
        auto it = slots_.find(name);
        if (it != slots_.end()) {
            return it->second;
        }
        Value slot = static_cast<Value>(program_.varNames.size());
        slots_[name] = slot;
        program_.varNames.push_back(name);
        return slot;
    }

    void pushOperand(const Operand& operand) {
        if (operand.isImmediate) {
            emit(OpCode::Push, operand.imm);
        } else {
            emit(OpCode::Load, slotFor(operand.name));
        }
    }

    // Emits a jump whose target address is not known yet.
    void emitJumpTo(OpCode opcode, const std::string& label) {
        size_t address = emit(opcode, 0);
        pendingJumps_.push_back({address, label});
    }

    void genInstr(const Instr& instr) {
        switch (instr.kind) {
            case InstrKind::Copy:
                pushOperand(instr.a);
                emit(OpCode::Store, slotFor(instr.dst));
                break;

            case InstrKind::Binary:
                pushOperand(instr.a);
                pushOperand(instr.b);
                emit(OpCode::Binary, 0, instr.op);
                emit(OpCode::Store, slotFor(instr.dst));
                break;

            case InstrKind::Label:
                // A label is just the address of the next instruction.
                labelAddress_[instr.label] = static_cast<Value>(program_.code.size());
                break;

            case InstrKind::Jump:
                emitJumpTo(OpCode::Jump, instr.label);
                break;

            case InstrKind::JumpIfZero:
                pushOperand(instr.a);
                emitJumpTo(OpCode::JumpIfZero, instr.label);
                break;

            case InstrKind::Print:
                pushOperand(instr.a);
                emit(OpCode::Print);
                break;
        }
    }

    // Second step: fill in the real addresses of all jumps.
    void patchJumps() {
        for (const PendingJump& jump : pendingJumps_) {
            program_.code[jump.address].operand = labelAddress_.at(jump.label);
        }
    }

    struct PendingJump {
        size_t address;
        std::string label;
    };

    VMProgram program_;
    std::unordered_map<std::string, Value> slots_;
    std::unordered_map<std::string, Value> labelAddress_;
    std::vector<PendingJump> pendingJumps_;
};

const char* binaryMnemonic(BinaryOp op) {
    switch (op) {
        case BinaryOp::Add: return "ADD";
        case BinaryOp::Sub: return "SUB";
        case BinaryOp::Mul: return "MUL";
        case BinaryOp::Div: return "DIV";
        case BinaryOp::Lt:  return "LT";
        case BinaryOp::Gt:  return "GT";
        case BinaryOp::Le:  return "LE";
        case BinaryOp::Ge:  return "GE";
        case BinaryOp::Eq:  return "EQ";
        case BinaryOp::Ne:  return "NE";
    }
    return "?";
}

}  // namespace

VMProgram generateCode(const IRProgram& ir) {
    CodeGenerator generator;
    return generator.run(ir);
}

void printAssembly(const VMProgram& program, std::ostream& out) {
    for (size_t i = 0; i < program.code.size(); i++) {
        const Bytecode& ins = program.code[i];
        out << std::setw(4) << i << "  ";

        switch (ins.opcode) {
            case OpCode::Push:
                out << "PUSH  " << ins.operand;
                break;
            case OpCode::Load:
                out << "LOAD  " << program.varNames[ins.operand];
                break;
            case OpCode::Store:
                out << "STORE " << program.varNames[ins.operand];
                break;
            case OpCode::Binary:
                out << binaryMnemonic(ins.binOp);
                break;
            case OpCode::Jump:
                out << "JMP   " << ins.operand;
                break;
            case OpCode::JumpIfZero:
                out << "JZ    " << ins.operand;
                break;
            case OpCode::Print:
                out << "PRINT";
                break;
            case OpCode::Halt:
                out << "HALT";
                break;
        }
        out << "\n";
    }
}

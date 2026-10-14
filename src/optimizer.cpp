#include "optimizer.h"

#include <unordered_map>

namespace {

// Pass 1: constant propagation and folding.
//
// We walk the code in order and remember which names currently hold a known
// constant. Labels are places where several paths can join (e.g. loop
// tops), so we forget everything there to stay correct.
// Returns true if the program was changed.
bool propagateAndFoldConstants(IRProgram& program) {
    bool changed = false;
    std::unordered_map<std::string, Value> known;
    IRProgram result;

    // Replace a variable operand by its known constant value.
    auto substitute = [&](Operand& operand) {
        if (operand.isImmediate) {
            return;
        }
        auto it = known.find(operand.name);
        if (it != known.end()) {
            operand = Operand::immediate(it->second);
            changed = true;
        }
    };

    for (Instr instr : program) {
        switch (instr.kind) {
            case InstrKind::Label:
                known.clear();
                break;

            case InstrKind::Jump:
                break;

            case InstrKind::Print:
                substitute(instr.a);
                break;

            case InstrKind::Copy:
                substitute(instr.a);
                if (instr.a.isImmediate) {
                    known[instr.dst] = instr.a.imm;
                } else {
                    known.erase(instr.dst);
                }
                break;

            case InstrKind::Binary: {
                substitute(instr.a);
                substitute(instr.b);

                bool bothConstant = instr.a.isImmediate && instr.b.isImmediate;
                bool divideByZero = instr.op == BinaryOp::Div && instr.b.imm == 0;

                if (bothConstant && !divideByZero) {
                    // Fold: "t = 2 * 3" becomes "t = 6".
                    Value folded = applyOp(instr.op, instr.a.imm, instr.b.imm);
                    instr.kind = InstrKind::Copy;
                    instr.a = Operand::immediate(folded);
                    known[instr.dst] = folded;
                    changed = true;
                } else {
                    known.erase(instr.dst);
                }
                break;
            }

            case InstrKind::JumpIfZero:
                substitute(instr.a);
                if (instr.a.isImmediate) {
                    changed = true;
                    if (instr.a.imm == 0) {
                        // Condition is always false: always jump.
                        instr.kind = InstrKind::Jump;
                    } else {
                        // Condition is always true: never jump, drop it.
                        continue;
                    }
                }
                break;
        }
        result.push_back(instr);
    }

    program = std::move(result);
    return changed;
}

// Pass 2: remove temporaries that are computed but never used.
bool removeDeadTemporaries(IRProgram& program) {
    std::unordered_map<std::string, int> useCount;

    auto countUse = [&](const Operand& operand) {
        if (!operand.isImmediate) {
            useCount[operand.name]++;
        }
    };

    for (const Instr& instr : program) {
        switch (instr.kind) {
            case InstrKind::Binary:
                countUse(instr.a);
                countUse(instr.b);
                break;
            case InstrKind::Copy:
            case InstrKind::Print:
            case InstrKind::JumpIfZero:
                countUse(instr.a);
                break;
            default:
                break;
        }
    }

    IRProgram result;
    bool changed = false;
    for (const Instr& instr : program) {
        bool definesValue = instr.kind == InstrKind::Copy || instr.kind == InstrKind::Binary;
        if (definesValue && isTemp(instr.dst) && useCount[instr.dst] == 0) {
            changed = true;  // drop this instruction
            continue;
        }
        result.push_back(instr);
    }

    program = std::move(result);
    return changed;
}

}  // namespace

void optimize(IRProgram& program) {
    bool changed = true;
    while (changed) {
        bool a = propagateAndFoldConstants(program);
        bool b = removeDeadTemporaries(program);
        changed = a || b;
    }
}

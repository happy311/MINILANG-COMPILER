// optimizer.h - Stage 4: IR -> better IR.
//
// Passes (repeated until nothing changes):
//   1. Constant propagation + folding   (x = 2; y = x + 3  ->  y = 5)
//   2. Dead temporary elimination       (removes unused %t values)
#pragma once

#include "ir.h"

void optimize(IRProgram& program);

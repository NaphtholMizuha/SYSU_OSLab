#ifndef OS_MODULES_H
#define OS_MODULES_H

#include "interrupt.h"
#include "std_io.h"
#include "program.h"
#include "memory_maneger.h"
#include "tss.h"

extern InterruptManeger interrupt_maneger;
extern Std_IO stdio;
extern ProgramManeger program_maneger;
extern MemoryManeger memory_maneger;
extern TSS tss;

#endif
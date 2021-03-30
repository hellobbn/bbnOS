//===- global.c - global variables ---------------------------------------===//
//
// Defines global variables.
//
//===---------------------------------------------------------------------===//
#define GLOBAL_VAR_HERE

#include "global.h"
#include "const.h"
#include "dt.h"
#include "interrupt.h"
#include "thread.h"

/// Demo task table
///{
PUBLIC TASK task_table[MAX_THREAD] = {{testA, STACK_SIZE_TESTA, "TestA"},
                               {testB, STACK_SIZE_TESTB, "testB"},
                               {testC, STACK_SIZE_TESTC, "testC"}};
///}

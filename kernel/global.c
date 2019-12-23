#define GLOBAL_VAR_HERE

#include "const.h"
#include "thread.h"
#include "dt.h"
#include "i8259.h"
#include "global.h"

// task table
EXTERN TASK task_table[MAX_THREAD] = {{testA, STACK_SIZE_TESTA, "TestA"},
                                      {testB, STACK_SIZE_TESTB, "testB"}};
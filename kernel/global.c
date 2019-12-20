#include "const.h"
#include "thread.h"

// task-table
PUBLIC TASK task_table[MAX_THREAD] = {{testA, STACK_SIZE_TESTA, "TestA"},
                                      {testB, STACK_SIZE_TESTB, "testB"}};
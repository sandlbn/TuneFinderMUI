#include <clib/exec_protos.h>
#include <exec/memory.h>
#include <stdlib.h>
#include <string.h>

void *allocate(size_t size, int type) { return AllocVec(size, MEMF_CLEAR); }

void deallocate(void *ptr, int type) { FreeVec(ptr); }


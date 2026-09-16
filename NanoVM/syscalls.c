#include "syscalls.h"
#include <inttypes.h>
#include <unistd.h>

int64_t nanovm_syscall_printi(NanoVMCpu *cpu) {
    int64_t value = cpu->registers[Reg4];
    printf("%" PRId64 "\n", value);
    return 0;
}


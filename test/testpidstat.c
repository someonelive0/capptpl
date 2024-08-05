#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pidstat.h"


int main()
{
    sys_info();
    resource_usage();

    uint64_t syscpuocc = sys_cpu_occupy();
    printf("sys cpu occ %zu\n\n", syscpuocc);

    uint64_t sysmem = sys_mem();
    printf("sys mem %zu\n\n", sysmem);

    pid_t pid = getpid();
    uint64_t pidcpuocc = pid_cpu_occupy(pid);
    printf("proc cpu occ %zu\n\n", pidcpuocc);

    uint64_t pidmem = pid_mem(pid);
    printf("proc mem %zu\n\n", pidmem);

    float usage = pid_mem_ratio(pid);
    printf("\nproc mem usage %f\n\n", usage);

    usage = pid_cpu_ratio(pid);
    printf("\nproc cpu usage %f\n\n", usage);

    exit(0);
}

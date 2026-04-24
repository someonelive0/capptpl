#include "stats_os.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int test_os_cpu_usage();
int test_proc_cpu_usage(pid_t pid);
int test_proc_mem_usage(pid_t pid);
int test_disk_usage(const char* path);


int main(int argc, char *argv[]) {
    pid_t pid = 0;
    if (argc > 1) {
        pid = atoi(argv[1]);
    } else {
        pid = getpid();
    }

    test_os_cpu_usage();

    test_proc_cpu_usage(pid);

    test_proc_mem_usage(pid);

    test_disk_usage(NULL);
}

int test_os_cpu_usage() {
    printf("test_os_cpu_usage\n");
    cpu_occupy_t cpu_stat1, cpu_stat2;
    memset(&cpu_stat1, 0, sizeof(cpu_occupy_t));
    memset(&cpu_stat2, 0, sizeof(cpu_occupy_t));

    for(int i=0; i<5; i++) {
        os_cpu_occupy(&cpu_stat1);
        sleep(1); // 等待1秒

        if (0 == os_cpu_occupy(&cpu_stat2)) {
            double usage = os_cpu_usage(&cpu_stat1, &cpu_stat2);
            printf("OS CPU占用率: %.2f%%\n", usage);
        }
    }

    return 0;
}

int test_proc_cpu_usage(pid_t pid) {
    printf("test_proc_cpu_usage of pid %d\n", pid);
    uint32_t proc_time1 = 0, proc_time2 = 0;
    cpu_occupy_t cpu_stat1, cpu_stat2;
    memset(&cpu_stat1, 0, sizeof(cpu_occupy_t));
    memset(&cpu_stat2, 0, sizeof(cpu_occupy_t));

    for(int i=0; i<5; i++) {
        os_cpu_occupy(&cpu_stat1);
        proc_cpu_time(pid, &proc_time1);
        // printf("proc_time1: %d\n", proc_time1);

        sleep(1); // 等待1秒

        if (0 == os_cpu_occupy(&cpu_stat2)) {
            proc_cpu_time(pid, &proc_time2);
            // printf("proc_time2: %d\n", proc_time2);

            double usage = proc_cpu_usage(proc_time1, proc_time2,
                &cpu_stat1, &cpu_stat2);
            printf("process [%d] CPU占用率: %.2f%%\n", pid, usage);
        }
    }

    return 0;
}

/**
 * mem in kB
 */
int test_proc_mem_usage(pid_t pid) {
    printf("test_proc_mem_usage of pid %d\n", pid);
    uint32_t proc_rss = 0;
    mem_stat_t mem_stat;
    memset(&mem_stat, 0, sizeof(mem_stat_t));

    os_mem_stats(&mem_stat);
    proc_mem_rss(pid, &proc_rss);
    // printf("process %d proc_rss: %d\n", pid, proc_rss);

    double usage = calculate_mem_usage(mem_stat.total, proc_rss);
    printf("process [%d] rss=%d kB, mem占用率: %.2f%%\n", pid, proc_rss, usage);

    return 0;
}

/**
 * when path is NULL, default get root '/'
 */
int test_disk_usage(const char* path) {
    printf("test_disk_usage\n");
    uint32_t total_blocks = 0, used_blocks = 0, block_size = 0;

    os_disk_stats(path, &total_blocks, &used_blocks, &block_size);
    printf("disk [%s], total=%d blocks, used=%d blocks, block_size=%d bytes\n",
        path, total_blocks, used_blocks, block_size);

    double usage = calculate_disk_usage(total_blocks, used_blocks);
    printf("disk [%s], disk占用率: %.2f%%\n", path, usage);

    return 0;
}

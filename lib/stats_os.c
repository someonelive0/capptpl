// gcc 

#include "stats_os.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/sysinfo.h>    // for get_nprocs()
#include <sys/statfs.h>        // for statvfs()
#endif

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif


// calculate cpu usage from prev to curr
double os_cpu_usage(const cpu_occupy_t *prev, const cpu_occupy_t *curr) {
    double prev_total = (double)(prev->user + prev->nice + prev->system 
                        + prev->idle + prev->iowait + prev->irq + prev->softirq);
    double curr_total = (double)(curr->user + curr->nice + curr->system 
                        + curr->idle + curr->iowait + curr->irq + curr->softirq);
    double idle_diff = (double)(curr->idle - prev->idle);
    if ((curr_total - prev_total) != 0) {
        return 100.0 - (idle_diff / (curr_total - prev_total)) * 100.0;
    }
    return 0.0;
}

// get cpu occupy of os, return 0 if success
int os_cpu_occupy(cpu_occupy_t *cpu_stat) {
#if defined(__linux__)
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return -1;
    fscanf(fp, "%s %u %u %u %u %u %u %u",
            cpu_stat->name, &cpu_stat->user, &cpu_stat->nice, &cpu_stat->system,
            &cpu_stat->idle, &cpu_stat->iowait, &cpu_stat->irq, &cpu_stat->softirq);
    fclose(fp);

#elif defined(WIN32)
    // TODO: Implement for Windows
#endif

    return 0;
}


// 向后偏移14项，正好是utime
#define PROCESS_ITEM 14

/**
    get cpu occupy of pid, return -1 if failed
    获取/proc/{pid}/stat中CPU的使用信息，主要是
    (14) utime  %lu
    (15) stime  %lu
    (16) cutime  %ld
    (17) cstime  %ld

    From /proc/<pid>/stat
            0    1    2    3     4    5       6   7 8 9  11  13   15
    3770 (cat) R 3718 3770 3718 34818 3770 4202496 214 0 0 0 0 0 0 0 20
    16  18     19      20 21                   22      23      24              25
    0 1 0 298215 5750784 81 18446744073709551615 4194304 4242836 140736345340592
                26
    140736066274232 140575670169216 0 0 0 0 0 0 0 17 0 0 0 0 0 0
*/
int proc_cpu_time(pid_t pid, uint32_t* cpu_time) {
    char filename[64];
    snprintf(filename, sizeof(filename) - 1, "/proc/%d/stat", pid);
    filename[sizeof(filename) - 1] = '\0';

    FILE *fd = fopen(filename, "r");
    if (!fd) return -1;

    char buf[1024];
    fgets(buf, sizeof(buf)-1, fd);
    buf[sizeof(buf)-1] = '\0';
    // fprintf(stdout, "%s: %s", filename, buf);
    fclose(fd);

    // 向后偏移，直至utime项
    int count = 1;
    const char *p = buf;
    while (*p != '\0') {
        if (*p++ == ' ') {
            count++;
            if (count == PROCESS_ITEM) break;
        }
    }
    if (*p == '\0') return 0;

    uint32_t utime = 0;    // user time
    uint32_t stime = 0;    // kernel time
    uint32_t cutime = 0;   // all user time
    uint32_t cstime = 0;   // all dead time
    sscanf(p, "%u %u %u %u", &utime, &stime, &cutime, &cstime);
    // fprintf(stdout, "proc[%d]: %u %u %u %u = %u\n",
    //     pid, utime, stime, cutime, cstime, utime + stime + cutime + cstime);
    *cpu_time = (utime + stime + cutime + cstime);
    return 0;
}

/**
 * calculate cpu usage of pid from prev to curr
 * 获取CPU利用率，如CPU利用率为60%，则返回60
 */
double proc_cpu_usage(uint32_t proc_time1, uint32_t proc_time2,
    const cpu_occupy_t *os_occupy1, const cpu_occupy_t *os_occupy2)
{
    // 计算CPU利用率：使用时间/总时间
    uint32_t sys_total1 = os_occupy1->user + os_occupy1->nice 
                        + os_occupy1->system + os_occupy1->idle;
    uint32_t sys_total2 = os_occupy2->user + os_occupy2->nice 
                        + os_occupy2->system + os_occupy2->idle;

    double used = 0.0;
    if (sys_total1 < sys_total2) {
        // 多线程，需要乘CPU core数量
        used = cpu_number() * 100 
            * (proc_time2 - proc_time1) / (sys_total2 - sys_total1);
    }

    return used;
}

// get cpu processes number
int cpu_number() {
#ifdef _WIN32
    return 1; // TODO: Implement for Windows
#else
    return get_nprocs();
#endif
}


// calculate mem usage from total and used in kB
double calculate_mem_usage(uint32_t total, uint32_t used) {
    if (total != 0) {
        return (double)used / (double)total * 100.0;
    }
    return 0.0;
}

/**
 * get mem stats of os, return 0 if success
 * another method is call getrusage(RUSAGE_SELF, &rusage)
 */
int os_mem_stats(mem_stat_t* mem_stat) {
#if defined(__linux__)
    memset(mem_stat, 0, sizeof(mem_stat_t));

    char buf[128] = {0};
    char name[64] = {0};
    char unit[64] = {0}; // default 'kB'

    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;

    fgets(buf, sizeof(buf)-1, fp);
    sscanf(buf, "%s %u %s", name, &mem_stat->total, unit);
    fgets(buf, sizeof(buf)-1, fp);
    sscanf(buf, "%s %u %s", name, &mem_stat->free, unit);
    fgets(buf, sizeof(buf)-1, fp);
    sscanf(buf, "%s %u %s", name, &mem_stat->avaliable, unit);
    fclose(fp);
    // fprintf(stdout, "mem: %u %u %u\n", mem_stat->total, mem_stat->free, mem_stat->avaliable);

#elif defined(_WIN32)
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    GlobalMemoryStatusEx(&mem_status);
    mem_stat->total = mem_status.ullTotalPhys / 1024; // KB
    // fprintf(stdout, "mem: %u %u %u\n", mem_stat->total, mem_stat->free, mem_stat->avaliable);

#endif // defined(_WIN32)

    return 0;
}

/**
    get mem stats of pid, return 0 if success
    解析/proc/{pid}/statm文件，获取内存信息，前两项为
    size (pages) 任务虚拟地址空间的大小 VmSize/4，单位page number
    resident(pages) 应用程序正在使用的物理内存的大小 VmRSS/4，单位page number
*/
int proc_mem_rss(pid_t pid, uint32_t *proc_rss) {
#if defined(__linux__)
    char filename[64];
    snprintf(filename, sizeof(filename) - 1, "/proc/%d/statm", pid);
    filename[sizeof(filename) - 1] = '\0';

    char buf[128] = {0};
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;
    fgets(buf, sizeof(buf)-1, fp);
    fclose(fp);

    uint32_t size = 0;
    uint32_t resident = 0;
    sscanf(buf, "%u %u", &size, &resident);
    // fprintf(stdout, "proc mem: %u %u\n", size, resident);

    // page_size in kB mostly is 4
    uint32_t page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    *proc_rss = resident * page_size_kb;

#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    *proc_rss = pmc.WorkingSetSize / 1024; // KB

#endif // defined(_WIN32)

    return 0;
}


// calculate disk usage from total and used in blocks
double calculate_disk_usage(uint32_t total, uint32_t used) {
    if (total != 0) {
        return (double)used / (double)total * 100.0;
    }
    return 0.0;
}

/**
 * get disk stats of os root '/', return 0 if success
 * block_size is in bytes
 */
int os_disk_stats(const char* path,
    uint32_t *total_blocks, uint32_t *used_blocks, uint32_t *block_size) {
#if defined(__linux__)
    if (NULL == path) path = "/"; // default get root '/'
    struct statfs disk_stat;
    int rc = statfs(path, &disk_stat);
    if (rc < 0) {
        return -1;
    }

    *total_blocks = disk_stat.f_blocks;
    *used_blocks = disk_stat.f_blocks - disk_stat.f_bavail;
    *block_size = disk_stat.f_bsize;

#elif defined(_WIN32)
    ULARGE_INTEGER available_bytes, total_bytes, free_bytes;
    if (NULL == path) path = "C:\\"; // default get root 'C:\\'

    // 获取 C 盘的磁盘信息
    if (!GetDiskFreeSpaceEx("C:\\", &available_bytes, &total_bytes, &free_bytes)) {
        // printf("获取磁盘信息失败，错误代码: %lu\n", GetLastError());
        return -1;
    }

    // printf("total_bytes: %llu bytes\n", total_bytes.QuadPart);
    // printf("available_bytes: %llu bytes\n", available_bytes.QuadPart);

    *total_blocks = total_bytes.QuadPart / 4096;
    *used_blocks = (total_bytes.QuadPart - available_bytes.QuadPart)/ 4096;
    *block_size = 4096;

#endif // defined(_WIN32)

    return 0;
}

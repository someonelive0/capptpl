#ifndef STATS_OS_H
#define STATS_OS_H

#include <stdint.h>
#include <sys/types.h>


/**
 * cpu occupy of os, read /proc/stat
 */
typedef struct cpu_occupy_ {
   char     name[20];   // 设备名，一般值为 cpu
   uint32_t user;       // 用户态的CPU时间（单位:0.01s）
   uint32_t nice;       // nice值为负的进程所占用的CPU时间（单位:0.01s）
   uint32_t system;     // 内核态占用的CPU时间（单位:0.01s）
   uint32_t idle;       // 除IO等待时间意外其他等待时间（单位:0.01s）
   uint32_t iowait;
   uint32_t irq;
   uint32_t softirq;
} cpu_occupy_t;

// calculate cpu usage from prev to curr
double os_cpu_usage(const cpu_occupy_t *prev, const cpu_occupy_t *curr);

// get cpu occupy of os, return 0 if success
int os_cpu_occupy(cpu_occupy_t *cpu_stat);

// get cpu occupy of pid, return -1 if failed
int proc_cpu_time(pid_t pid, uint32_t* cpu_time);

/**
 * calculate cpu usage of pid from prev to curr
 * 获取CPU利用率，如CPU利用率为60%，则返回60
 */
double proc_cpu_usage(uint32_t proc_time1, uint32_t proc_time2,
   const cpu_occupy_t *os_occupy1, const cpu_occupy_t *os_occupy2);

// get cpu processes number
int cpu_number();


/**
 * mem usage of os or pid, read /proc/meminfo
 * size unit: kB
 */
typedef struct mem_stat_ {
   uint32_t total;      // 总内存
   uint32_t free;       // 空闲内存
   uint32_t avaliable;  // 可用内存，约等于 MemFree + Buffer + Catch
   // uint32_t used;       // 已使用内存
} mem_stat_t;

// calculate mem usage from total and used in kB
double calculate_mem_usage(uint32_t total, uint32_t used);

// get mem stats of os, return 0 if success
int os_mem_stats(mem_stat_t* mem_stat);

// get mem rss of pid in kB, return 0 if success
int proc_mem_rss(pid_t pid, uint32_t *proc_rss);


// calculate disk usage from total and used in blocks
double calculate_disk_usage(uint32_t total, uint32_t used);

/**
 * get disk stats of os root '/', return 0 if success
 * block_size is in bytes
 */
int os_disk_stats(const char* path,
   uint32_t *total_blocks, uint32_t *used_blocks, uint32_t *block_size);

#endif // STATS_OS_H

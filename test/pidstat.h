/*
 * system and process cpu, mem occupy/useage
 */
#include <stdint.h>
#include <sys/types.h>


/*
 * system cpu occupy/usage
 */
struct sys_cpu_occupy {
    uint32_t user;  //从系统启动开始累计到当前时刻，处于用户态的运行时间，不包含 nice值为负进程。
    uint32_t nice;  //从系统启动开始累计到当前时刻，nice值为负的进程所占用的CPU时间
    uint32_t system;//从系统启动开始累计到当前时刻，处于核心态的运行时间
    uint32_t idle;  //从系统启动开始累计到当前时刻，除IO等待时间以外的其它等待时间iowait (12256) 从系统启动开始累计到当前时刻，IO等待时间(since 2.5.41)
};

/*
 * process/pid cpu occupy/usage
 */
struct pid_cpu_occupy {
    pid_t pid;        //pid号
    uint32_t utime;  //该任务在用户态运行的时间，单位为jiffies
    uint32_t stime;  //该任务在核心态运行的时间，单位为jiffies
    uint32_t cutime; //所有已死线程在用户态运行的时间，单位为jiffies
    uint32_t cstime;  //所有已死在核心态运行的时间，单位为jiffies
};

/*
 * system cpu and mem stat
 */
int sys_info();
int resource_usage();
uint64_t  sys_cpu_occupy();       //获取总的CPU时间
uint64_t  sys_mem();              //获取系统总内存

/*
 * process with pid cpu and mem stat
 */
uint64_t  pid_cpu_occupy(pid_t p);       //获取进程的CPU时间
uint64_t  pid_mem(pid_t p);              //获取porcess RSS内存
float     pid_cpu_ratio(pid_t p);        //获取进程CPU占有率
float     pid_mem_ratio(pid_t p);        //获取进程内存占有率

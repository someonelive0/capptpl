/*
 * get_cpu.c
 * only work on linux
 */
#include "pidstat.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/resource.h>

#define PROCESS_ITEM 14  //进程CPU时间开始的项数


/*
 * system cpu and mem stat
 */
int sys_info()
{
    char info_buff[100];
    struct sysinfo s_info;
    if (0 != sysinfo(&s_info)) {
        return -1;
    }
    time_t time_up = time(NULL) - s_info.uptime;

    sprintf(info_buff,"总内存: %.ld M",s_info.totalram/1024/1024);
    printf("%s\n",info_buff);

    sprintf(info_buff,"未使用内存: %.ld M",s_info.freeram/1024/1024);
    printf("%s\n",info_buff);

    sprintf(info_buff,"交换区总内存: %.ld M",s_info.totalswap/1024/1024);
    printf("%s\n",info_buff);

    sprintf(info_buff,"交换区未使用内存: %.ld M",s_info.freeswap/1024/1024);
    printf("%s\n",info_buff);

    sprintf(info_buff,"系统运行时间: %.ld 分钟",s_info.uptime/60);
    printf("%s\n",info_buff);

    printf("uptime: %zu, %s\n", time_up, asctime(localtime( &time_up )));

    printf("\n\n");
    putc('\n', stdout);

    return 0;
}

int resource_usage()
{
    struct rusage usage;
    int rc = getrusage(RUSAGE_SELF, &usage);
    if (0 != rc) {
        return rc;
    }

    printf("%s: %.3fms\n", "ru_utime", (usage.ru_utime.tv_sec * 1000.0 + usage.ru_utime.tv_usec / 1000.0)); // 返回进程在用户模式下的执行时间，以timeval结构的形式返回（该结构体在bits/timeval中声明）；
	printf("%s: %.3fms\n", "ru_stime", (usage.ru_stime.tv_sec * 1000.0 + usage.ru_stime.tv_usec / 1000.0)); // 返回进程在内核模式下的执行时间，以timeval结构的形式返回（该结构体在bits/timeval中声明）；
	printf("%s: %.3fM\n", "ru_maxrss", (usage.ru_maxrss / 1024.0)); // 返回rss（实际使用物理内存，包含共享库占用的内存）的大小，单位为KB；当who被指定为RUSAGE_CHILDREN时，返回各子进程rss的大小中最大的一个，而不是进程树中最大的rss；
	printf("%s: %ld\n", "ru_ixrss", usage.ru_ixrss);
	printf("%s: %ld\n", "ru_idrss", usage.ru_idrss);
	printf("%s: %ld\n", "ru_isrss", usage.ru_isrss);
	printf("%s: %ld\n", "ru_minflt 缺页中断的次数 ", usage.ru_minflt); // 缺页中断的次数，且处理这些中断不需要进行I/O，不需要进行I/O操作的原因是系统使用reclaiming的方式在物理内存中得到了之前被淘汰但是未被修改的页框。（第一次访问bss段时也会产生这种类型的缺页中断）；
	printf("%s: %ld\n", "ru_majflt 缺页中断的次数 ", usage.ru_majflt); // 缺页中断的次数，且处理这些中断需要进行I/O；
	printf("%s: %ld\n", "ru_nswap", usage.ru_nswap);
	printf("%s: %ld\n", "ru_inblock", usage.ru_inblock);
	printf("%s: %ld\n", "ru_oublock", usage.ru_oublock);
	printf("%s: %ld\n", "ru_msgsnd", usage.ru_msgsnd);
	printf("%s: %ld\n", "ru_msgrcv", usage.ru_msgrcv);
	printf("%s: %ld\n", "ru_nsignals", usage.ru_nsignals);
	printf("%s: %ld\n", "ru_nvcsw 自愿上下文切换的次数 ", usage.ru_nvcsw); // 因进程自愿放弃处理器时间片而导致的上下文切换的次数（通常是为了等待请求的资源）；
	printf("%s: %ld\n", "ru_nivcsw 抢断上下文切换的次数 ", usage.ru_nivcsw); // 因进程时间片使用完毕或被高优先级进程抢断导致的上下文切换的次数；
    printf("\n");

    return 0;
}

//获取总的CPU时间
uint64_t sys_cpu_occupy()
{
    FILE *fp;
    char buff[1024] = {0};
    struct sys_cpu_occupy t;

    fp = fopen ("/proc/stat", "r");
    if (NULL == fp) return 0;

    fgets (buff, sizeof(buff), fp);
    /*下面是将buff的字符串根据参数format后转换为数据的结果存入相应的结构体参数 */
    char name[16];
    sscanf (buff, "%s %u %u %u %u", name, &t.user, &t.nice,&t.system, &t.idle);

    printf ("====%s:%u %u %u %u====\n", name, t.user, t.nice,t.system, t.idle);
    fclose(fp);

    return (t.user + t.nice + t.system + t.idle);
}

/* 
 * 获取系统总内存
$ cat /proc/meminfo
MemTotal:       32218136 kB
MemFree:        21395652 kB
HighTotal:             0 kB
HighFree:              0 kB
LowTotal:       32218136 kB
LowFree:        21395652 kB
SwapTotal:       2031616 kB
SwapFree:        2024776 kB
*/
uint64_t sys_mem()
{
    char* filename = "/proc/meminfo";

    FILE *fp;
    char line[256] = {0};
    fp = fopen (filename, "r");
    if (NULL == fp) return 0;

    //获取memtotal:总内存占用大小
    char name[32];
    uint32_t memtotal;
    fgets (line, sizeof(line), fp);
    sscanf (line, "%s %d", name, &memtotal);
    printf ("====%s: %d kb====\n", name, memtotal);
    fclose(fp);
    return memtotal;
}


/*
 * process with pid cpu and mem stat
 * 获取进程的CPU时间
 */
const char* get_items(const char* buffer, int ie); //取得缓冲区指定项的起始地址

uint64_t  pid_cpu_occupy(pid_t p)
{
    char file[64] = {0};
    struct pid_cpu_occupy t;

    FILE *fp;
    char line[1024] = {0};
    sprintf(file,"/proc/%d/stat", p);//文件中第11行包含着

    fprintf (stderr, "current pid:%d\n", p);
    fp = fopen (file, "r");
    if (NULL == fp) return 0;
    fgets (line, sizeof(line), fp);

    sscanf(line,"%u",&t.pid);//取得第一项
    const char* q = get_items(line, PROCESS_ITEM);//取得从第14项开始的起始指针
    sscanf(q,"%u %u %u %u",&t.utime,&t.stime,&t.cutime,&t.cstime);//格式化第14,15,16,17项

    printf ("==== pid %u:%u %u %u %u====\n", t.pid, t.utime,t.stime,t.cutime,t.cstime);
    fclose(fp);
    return (t.utime + t.stime + t.cutime + t.cstime);
}

//获取porcess RSS内存
uint64_t pid_mem(const pid_t p)
{
    FILE *fp;
    char file[64] = {0};
    char line[256] = {0};
    sprintf(file,"/proc/%d/status", p);

    fprintf (stderr, "current pid:%d\n", p);
    fp = fopen (file, "r");
    if (NULL == fp) return 0;

    char name[32];
    int vmrss;
    while (fgets (line, sizeof(line), fp)) {
        if (strncmp(line, "VmRSS:", 6) == 0) { //读到VmRSS:
        sscanf (line, "%s %d", name,&vmrss);
        fprintf (stderr, "====%s: %d kb====\n", name,vmrss);
            break;
        }
    }

    fclose(fp);
    return vmrss;
}

//获取进程CPU占有率
float pid_cpu_ratio(pid_t p)
{
    uint64_t totalcputime1, totalcputime2;
    uint64_t procputime1, procputime2;
    totalcputime1 = sys_cpu_occupy();
    procputime1 = pid_cpu_occupy(p);
    usleep(500000);//延迟500毫秒
    totalcputime2 = sys_cpu_occupy();
    procputime2 = pid_cpu_occupy(p);
    float pcpu = 100.0*(procputime2 - procputime1)/(totalcputime2 - totalcputime1);
    fprintf(stderr,"====pcpu:%.6f\n====",pcpu);
    return pcpu;
}

//获取进程内存占有率
float pid_mem_ratio(pid_t p)        
{
    uint64_t rss = pid_mem(p);
    uint64_t total = sys_mem();
    float occupy = (rss*1.0)/(total*1.0);
    fprintf(stderr,"====process mem occupy:%.6f\n====",occupy);
    return occupy;
}

const char* get_items(const char* buffer,int ie)
{
    const char* p = buffer; //指向缓冲区
    size_t len = strlen(buffer);
    int count = 0;//统计空格数
    if (1 == ie || ie < 1) {
        return p;
    }
    int i;

    for (i=0; i<len; i++) {
        if (' ' == *p) {
            count++;
            if (count == ie-1) {
                p++;
                break;
            }
        }
        p++;
    }

    return p;
}

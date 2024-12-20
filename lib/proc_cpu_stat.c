#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


// From /proc/<pid>/stat
//            0    1    2    3     4    5       6   7 8 9  11  13   15
// 3770 (cat) R 3718 3770 3718 34818 3770 4202496 214 0 0 0 0 0 0 0 20
// 16  18     19      20 21                   22      23      24              25
//  0 1 0 298215 5750784 81 18446744073709551615 4194304 4242836 140736345340592
//              26
// 140736066274232 140575670169216 0 0 0 0 0 0 0 17 0 0 0 0 0 0

struct proc_stat
{
    char state;
    unsigned int ppid;
    unsigned int pgrp;
    unsigned int session;
    unsigned int tty_nr;
    unsigned int tpgid;
    unsigned int flags;

    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;

    unsigned long utime;
    unsigned long stime;
    unsigned long cutime;
    unsigned long cstime;

    unsigned long priority;
    unsigned long nice;
    unsigned long num_threads;
    unsigned long itrealvalue;
    unsigned long starttime;
};

int read_proc_stat(struct proc_stat* stat, pid_t pid)
{
    char buf[1024] = {0};
    sprintf(buf, "/proc/%d/stat", pid);
    FILE* fp = fopen(buf, "r");
    if (!fp) {
        return -1;
    }
    char* p = fgets(buf, sizeof(buf), fp);
    if (p == NULL) {
        fclose(fp);
        return -1;
    }
    // printf("pid %d stat line: %s\n", pid, buf);

    int rc = proc_stat_decode(stat, buf);
    // printf("proc_stat_decode %d: %u %u, %u %u\n", rc,
    //     stat->utime, stat->stime, stat->cutime, stat->cstime);

    fclose(fp);
    return rc;
}


char *jump2utime(const char *buf, unsigned int pos_space)
{
    const char* p = buf;
    size_t l = strlen(buf);
    unsigned int space_count = 0;

    for (size_t i = 0; i < l; i++) {
        /* buf begin with "14971 (dragent) S ...", program name maybe has space*/
        if (space_count == 0) {
            if (')' == *p) {
                space_count = 1;
            }
            p++;
            continue;
        }

        if (' ' == *p) {           /* 以空格为标记符进行识别 */
            space_count++;
            if (space_count + 1 == pos_space) { /* 全部个数都找完了 */
                p++;
                return (char*)p;
            }
        }
        p++;
    }

    // printf("pos %d, %d: %s\n", space_count, p-buf, p);

    return NULL; // failed jump to position, should return NULL
}


#define UTIME_POS 14
int proc_stat_decode(struct proc_stat* stat, const char* buf)
{
    const char* p = jump2utime(buf, UTIME_POS);
    if (NULL == p)
        return -1;
        
    int rc = sscanf(p, "%u %u %u %u", &stat->utime, &stat->stime, &stat->cutime, &stat->cstime);
    if (EOF == rc)
        return -1;

    return 0;
}

double cpu_usage(int user_ticks, int sys_ticks, double kPeriod, double kClockTicksPerSecond)
{
	return (user_ticks + sys_ticks) / (kClockTicksPerSecond * kPeriod); //CPU使用率计算
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s pid\n", argv[0]);
		return 1;
	}
	pid_t pid = atoi(argv[1]);

    struct proc_stat last_stat = {0};
    struct proc_stat stat = {0};
    time_t stat_time;
    time_t last_stat_time;

    int rc = 0;
    int had_stat = 0; // if had stat data.
    int user_ticks = 0;
    int sys_ticks = 0;
    const int clock_ticks = 100; //sysconf(_SC_CLK_TCK);
    int period = 0;
    while (1) {
        rc = read_proc_stat(&stat, pid);
        stat_time = time(NULL);
        if (rc == 0) {
            if (had_stat > 0) {
                user_ticks = stat.utime > last_stat.utime ? stat.utime - last_stat.utime : 0;	
                sys_ticks = stat.stime > last_stat.stime ? stat.stime - last_stat.stime : 0;
                period = stat_time - last_stat_time;
                printf("pid %d in %d seconds cpu usage:%.1f%%\n", pid, period,
                    cpu_usage(user_ticks, sys_ticks, period, clock_ticks) * 100);
            }
            had_stat = 1;
            last_stat_time = stat_time;
            memcpy(&last_stat, &stat, sizeof(struct proc_stat));
        }
        sleep(3);
    }

    return 0;
}

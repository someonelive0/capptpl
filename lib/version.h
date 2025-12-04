#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>
#include <time.h>

#define MY_VERSION "1.7.0"
#define SHOW_VERSION() printf("versoin %s\t build on %s %s\n", MY_VERSION, __DATE__, __TIME__)

#ifdef __linux__
#define SHOW_VERSION_LOCAL() { \
    struct tm t; \
    char date_time_str[64]; \
    strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &t); \
    strftime(date_time_str, sizeof(date_time_str), "%Y-%m-%dT%H:%M:%S", &t); \
    printf("versoin %s\t build on %s\n", MY_VERSION, date_time_str); \
}
#else
#define SHOW_VERSION_LOCAL() printf("versoin %s\t build on %s %s\n", MY_VERSION, __DATE__, __TIME__)
#endif

// const char *BANNER = 
// ""
// " █████╗ ██████╗ ██████╗ ████████╗██████╗ ██╗     "
// "██╔══██╗██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗██║     "
// "███████║██████╔╝██████╔╝   ██║   ██████╔╝██║     "
// "██╔══██║██╔═══╝ ██╔═══╝    ██║   ██╔═══╝ ██║     "
// "██║  ██║██║     ██║        ██║   ██║     ███████╗"
// "╚═╝  ╚═╝╚═╝     ╚═╝        ╚═╝   ╚═╝     ╚══════╝"
// "";

// read banner from file banner.txt
static inline int show_banner(const char* banner_file) {
    int ret = 0;
    char buf[100] = {0};

    FILE* fp = fopen(banner_file, "r");
    if (fp != NULL) {
        while(fgets(buf, sizeof(buf)-1, fp)) {
            printf("%s", buf);
        }
        fclose(fp);
    } else {
        ret = -1;
    }

    SHOW_VERSION_LOCAL(); // printf("versoin %s\n\n", MY_VERSION);

    return ret;
}

#endif // VERSION_H

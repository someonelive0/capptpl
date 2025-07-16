#ifndef VERSION_H
#define VERSION_H

#include <stdio.h>

#define MY_VERSION "1.5.0"
#define SHOW_VERSION() printf("versoin %s\n", MY_VERSION)

// int show_version() {
//     printf("versoin %s\n", MY_VERSION);
//     return 0;
// }


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

    printf("versoin %s\n\n", MY_VERSION);

    return ret;
}

#endif // VERSION_H

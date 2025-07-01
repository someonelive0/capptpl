#ifndef INIT_H
#define INIT_H

#include <time.h>


/*
 * struct app of this program, means apptpl
 */
struct app {
    time_t           run_time;
    struct config*   myconfig;
    struct capture*  captr;
    struct parser*   prsr;
    struct inputer*  inptr;
    struct worker*   wrkr;
};

int parse_args(int argc, const char** argv, int* debug, const char** config_filename);

#endif // INIT_H

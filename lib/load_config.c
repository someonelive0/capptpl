#include "load_config.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
// #ifdef __linux__
// #include <linux/limits.h> // PATH_MAX=4096
// #endif


#if 0
int ini_cb(void* arg, const char* section, const char* name, const char* value)
{
    UNUSED(arg);
    // configuration* pconfig = (configuration*)arg;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("global", "version")) {
        printf("ini global %s, name %s, value %s\n", section, name, value);
        // pconfig->version = atoi(value);
    } else if (MATCH("user", "name")) {
        // pconfig->name = strdup(value);
    } else if (MATCH("user", "email")) {
        // pconfig->email = strdup(value);
    } else {
        printf("ini section %s, name %s, value %s\n", section, name, value);
        // return 0;  /* unknown section/name, error */
    }
    return 1;
}
#endif

int load_config_ini(const char* filename, ini_handler ini_callback, void* arg)
{
    char* tpl_filename;
    int rc;

    rc = access(filename, F_OK);
    if (0 != rc) {
        printf("load_config ini file '%s' not exists\n", filename);
        tpl_filename = malloc(strlen(filename) + 5);
        strcpy(tpl_filename, filename);
        strcat(tpl_filename, ".tpl");
        rc = copy_file(tpl_filename, filename);
        free(tpl_filename);
        if (0 != rc) {
            printf("load_config copy ini tpl file '%s.tpl' to ini '%s' failed\n", filename, filename);
            return -1;
        }
    }

    if (ini_parse(filename, ini_callback, arg) < 0) {
        printf("load_config parse ini file failed '%s'\n", filename);
        return -1;
    }

    return 0;
}

int copy_file(const char* in_path, const char* out_path)
{
    size_t n;
    FILE* in=NULL, * out=NULL;
    char buf[64];
    int rc = 0;

    if ((in = fopen(in_path, "rb")) && (out = fopen(out_path, "wb")))
        while ((n = fread(buf, 1, sizeof(buf), in)) && fwrite(buf, 1, n, out));
    else rc = -1;

    if (in) fclose(in);
    if (out) fclose(out);
    return rc;
}

/*
 * try copy file from file.tpl
 */
int copy_file_from_tpl(const char* filename)
{
    char* tpl_filename;
    int rc;

    rc = access(filename, F_OK);
    if (0 != rc) {
        printf("file '%s' not exists, try copy from tpl\n", filename);
        tpl_filename = malloc(strlen(filename) + 5);
        strcpy(tpl_filename, filename);
        strcat(tpl_filename, ".tpl");
        rc = copy_file(tpl_filename, filename);
        free(tpl_filename);
        if (0 != rc) {
            printf("copy file '%s' from '%s.tpl' failed\n", filename, filename);
            return -1;
        }
    }

    return 0;
}

// change cwd to dir of exe
#define UNUSED(x) (void)(x)
int ch_exec_cwd(char* argv0)
{
    char path[4096] = {0}; // PATH_MAX=4096

#ifdef _WIN32
    strncpy(path, argv0, sizeof(path)-1);
#else
    UNUSED(argv0);
    if (-1 == readlink("/proc/self/exe", path, sizeof(path))) {
        printf("read self exe path failed: %d, %s\n", errno, strerror(errno));
        return -1;
    }
#endif

    char* pwd = dirname(path);
    if (0 != chdir(pwd)) {
        printf("change cwd to [%s] failed: %d, %s\n", pwd, errno, strerror(errno));
        return -1;
    }
    // printf ("chdir to %s\n", pwd);
    return 0;
}

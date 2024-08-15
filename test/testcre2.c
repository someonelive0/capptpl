/*
 * g++ -I../lib/cre2 ../lib/cre2/cre2.cpp -c
 * gcc -I../lib/cre2 cre2.o testcre2.c -lre2 -lstdc++ -o testcre2
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "cre2.h"


static int test_simple_re();
static cre2_options_t* mkopt();
static int match(cre2_regexp_t* rex);

static int test_set_re();
static int match_set(cre2_set* rex_set, int nset);
void* match_set_thread(void *arg);


int main()
{
    test_simple_re();

    test_set_re();

    return 0;
}


static int test_simple_re()
{
    cre2_regexp_t *   rex;
    cre2_options_t *  opt;
    const char *      pattern = "(ciao) (hello)";

    printf("\ntest_simple_re()\n");
    opt = mkopt();
    if (NULL == opt) {
        printf("cre2_opt_new failed\n");
        return -1;
    }
    cre2_opt_set_posix_syntax(opt, 1);

    rex = cre2_new(pattern, strlen(pattern), opt);
    if (rex) {
        if (!cre2_error_code(rex)) {
            match(rex);
        } else {
            printf("cre2_new failed: %d, %s\n",
                   cre2_error_code(rex), cre2_error_string(rex));
        }
        cre2_delete(rex);
    } else {
        /* rex memory allocation error */
    }
    cre2_opt_delete(opt);

    return 0;
}

static cre2_options_t* mkopt()
{
    cre2_options_t*  opt;
    cre2_encoding_t  encoding;

    opt = cre2_opt_new();
    if (NULL == opt) {
        printf("cre2_opt_new failed\n");
        return NULL;
    }

    // default UTF-8 == 1
    encoding = cre2_opt_encoding(opt);
    printf("cre2_opt_encoding: %d\n", encoding);
    cre2_opt_set_encoding(opt, CRE2_UTF8);
    printf("after set, cre2_opt_encoding: %d\n", cre2_opt_encoding(opt));

    // Restrict regexps to POSIX egrep syntax. Default is disabled.
    printf("cre2_opt_posix_syntax: %d\n", cre2_opt_posix_syntax(opt));
    printf("cre2_opt_longest_match: %d\n", cre2_opt_longest_match(opt));
    printf("cre2_opt_log_errors: %d\n", cre2_opt_log_errors(opt));
    printf("cre2_opt_literal: %d\n", cre2_opt_literal(opt));
    printf("cre2_opt_never_nl: %d\n", cre2_opt_never_nl(opt));
    printf("cre2_opt_dot_nl: %d\n", cre2_opt_dot_nl(opt));
    printf("cre2_opt_never_capture: %d\n", cre2_opt_never_capture(opt));
    printf("cre2_opt_case_sensitive: %d\n", cre2_opt_case_sensitive(opt));
    printf("cre2_opt_max_mem: %zu\n", cre2_opt_max_mem(opt));

    // Allow Perl’s \d, \s, \w, \D, \S, \W. Default is disabled.
    printf("cre2_opt_perl_classes: %d\n", cre2_opt_perl_classes(opt));

    // Allow Perl’s \b, \B (word boundary and not). Default is disabled.
    printf("cre2_opt_word_boundary: %d\n", cre2_opt_word_boundary(opt));

    // The patterns ^ and $ only match at the beginning and end of the text. Default is disabled.
    printf("cre2_opt_one_line: %d\n", cre2_opt_one_line(opt));

    // disable log syntax and execution errors to stderr
    cre2_opt_set_log_errors(opt, 0);

    printf("\n");

    return opt;
}

static int match(cre2_regexp_t* rex)
{
    const char *   text     = "ciao hello";
    int            text_len = strlen(text);
    int            nmatch   = 3;
    cre2_string_t  match[nmatch];

    cre2_match(rex, text, text_len, 0, text_len, CRE2_UNANCHORED,
                match, nmatch);

    /* prints: full match: ciao hello */
    printf("full match: ");
    fwrite(match[0].data, match[0].length, 1, stdout);
    printf("\n");

    /* prints: first group: ciao */
    printf("first group: ");
    fwrite(match[1].data, match[1].length, 1, stdout);
    printf("\n");

    /* prints: second group: hello */
    printf("second group: ");
    fwrite(match[2].data, match[2].length, 1, stdout);
    printf("\n");

    return 0;
}

/*
 * test set of regex.
 */
static int test_set_re()
{
    cre2_set *   rex_set;
    cre2_options_t *  opt;
    const char *      patterns[] = { "select", "insert", "update", NULL };
    char error[128];
    size_t errlen = sizeof(error);

    printf("\ntest_set_re()\n");
    opt = mkopt();
    if (NULL == opt) {
        printf("cre2_opt_new failed\n");
        return -1;
    }
    cre2_opt_set_posix_syntax(opt, 1);

    rex_set = cre2_set_new(opt, CRE2_UNANCHORED);
    if (rex_set) {
        if (!cre2_error_code(rex_set)) {
            int i;
            for (i=0; patterns[i] != NULL ;i++) {
                if (-1 == cre2_set_add(rex_set, patterns[i], strlen(patterns[i]), error, errlen)) {
                    printf("cre2_set_add '%s' failed: %s\n", patterns[i], error);
                    continue;
                }
                printf("cre2_set_add '%s'\n", patterns[i]);
            }

            if (cre2_set_compile(rex_set)) {
                match_set(rex_set, i);
                // match_set_thread((void*)rex_set);
            } else {
                printf("cre2_set_compile failed: %d, %s\n",
                   cre2_error_code(rex_set), cre2_error_string(rex_set));
            }
            // match(rex_set);
        } else {
            printf("cre2_set_new failed: %d, %s\n",
                   cre2_error_code(rex_set), cre2_error_string(rex_set));
        }
        cre2_set_delete(rex_set);
    } else {
        /* rex memory allocation error */
    }
    cre2_opt_delete(opt);

    return 0;
}

void* match_set_thread(void *arg)
{
    const char *   text     = "just sql of insert, and update, more is select haha!";
    int            text_len = strlen(text);
    int            nmatch   = 3; // we has 3 regex strings.
    int            match[nmatch];
    int            rc;
    cre2_set* rex_set = arg;

    printf("begin match_set_thread %zu\n", pthread_self());
    clock_t t0 = clock();
    for (int loop=0; loop<1000000; loop++) {
        rc = cre2_set_match(rex_set, text, text_len,
                    match, nmatch);
        if ((loop % 100000) == 0) {
            fprintf(stderr, "%zu matched: loop %d  --> %d\n",
                    pthread_self(), loop, match[0]);
            for (int i=0; i<rc; i++) { // not work in pthread. ???
                fprintf(stderr, "%zu matched: loop %d  --> %d\n",
                        pthread_self(), loop, match[i]);
            }
        }
    }
    clock_t t1 = clock();

    /* prints: full match: ciao hello */
    printf("cre2_set_match '%s': %d/%d, use %lu ms\n", text, rc, nmatch, t1-t0);
    fprintf(stderr, "----> %zu matched:  --> %d\n", pthread_self(), match[0]);
    for (int i=0; i<rc; i++) {
        fprintf(stderr, "----> %zu matched:  --> %d\n", pthread_self(), match[i]);
    }
    fflush(stdout);

    return ((void*)0);
}

static int match_set(cre2_set* rex_set, int nset)
{
    int t_num = 8;
    pthread_t tids[t_num];

    setbuf(stdout, NULL);

    printf("create %d thread to set match\n", t_num);
    for (int i=0; i< t_num; i++) {
        pthread_create(&tids[i], NULL, match_set_thread, ((void *)&rex_set));
    }

    // sleep(10);
    printf("wait %d thread to set match\n", t_num);
    for (int i=0; i< t_num; i++) {
        pthread_join(tids[i], NULL);
    }
    printf("end %d thread to set match\n", t_num);

    return 0;
}

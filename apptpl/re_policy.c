#include "re_policy.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#include "logger.h"
#include "load_config.h"


static cre2_options_t* mk_cre2_opt();


// create re_policy from policy filename.
int re_policy_create(struct re_policy* rep, const char* filename)
{
    cre2_set *        rex_set = NULL;
    cre2_options_t *  rex_opt = NULL;
    int               rule_num = 0;
    struct re_rule*   rules;
    // const char *      patterns[] = { "select", "insert", "update", NULL };
    char error[128];
    size_t errlen = sizeof(error);

    if (0 != copy_file_from_tpl(filename)) {
        LOG_ERROR ("regex_policy '%s' or '%s.tpl' not existed", filename, filename);
        return -1;
    }

    UT_array* lines = re_policy_load(filename);
    if (NULL == lines) {
        return -1;
    }
    rules = (struct re_rule*)malloc(utarray_len(lines) * sizeof(struct re_rule));

    rex_opt = mk_cre2_opt();
    if (NULL == rex_opt) {
        return -1;
    }

    rex_set = cre2_set_new(rex_opt, CRE2_UNANCHORED);
    if (NULL == rex_set) {
        LOG_ERROR ("cre2_set_new failed");
        goto err;
    }
    if (cre2_error_code(rex_set)) {
        LOG_ERROR ("cre2_set_new failed: %d, %s",
                   cre2_error_code(rex_set), cre2_error_string(rex_set));
        goto err;
    }

    // for (int i=0; patterns[i] != NULL ; i++) {
    //     if (-1 == cre2_set_add(rex_set, patterns[i], strlen(patterns[i]), error, errlen)) {
    //         LOG_ERROR ("cre2_set_add '%s' failed: %s", patterns[i], error);
    //         continue;
    //     }
    //     rule_num ++;
    //     LOG_DEBUG ("cre2_set_add '%s'", patterns[i]);
    // }
    char** p = NULL;
    while ( (p=(char**)utarray_next(lines, p))) {
        if (-1 == cre2_set_add(rex_set, *p, strlen(*p), error, errlen)) {
            LOG_ERROR ("cre2_set_add '%s' failed: %s", *p, error);
            continue;
        }
        rules[rule_num].index = rule_num;
        rules[rule_num].pattern = strdup(*p);
        rule_num ++;
        LOG_DEBUG ("cre2_set_add '%s'", *p);
    }

    if (!cre2_set_compile(rex_set)) {
        LOG_ERROR ("cre2_set_compile failed: %d, %s",
                   cre2_error_code(rex_set), cre2_error_string(rex_set));
        goto err;
    }

    rep->rex_opt = rex_opt;
    rep->rex_set = rex_set;
    rep->rule_num = rule_num;
    rep->rules = rules;
    utarray_free(lines);

    return 0;

err:
    if (rex_set) cre2_set_delete(rex_set);
    if (rex_opt) cre2_opt_delete(rex_opt);
    if (lines) utarray_free(lines);
    return -1;
}

int re_policy_destroy(struct re_policy* rep)
{
    if (rep->rex_set) {
        cre2_set_delete(rep->rex_set);
        rep->rex_set = NULL;
    }
    if (rep->rex_opt) {
        cre2_opt_delete(rep->rex_opt);
        rep->rex_opt = NULL;
    }
    for (int i=0; i<rep->rule_num; i++) {
        free(rep->rules[i].pattern);
    }
    free(rep->rules);
    return 0;
}

void re_policy_dump(struct re_policy* rep)
{
    LOG_INFO ("rule num: %d", rep->rule_num);
}

// return matched number, match[] contain matched regex index
int re_policy_match(struct re_policy* rep, char* text, size_t text_len, int match[])
{
    int            nmatch   = rep->rule_num; // must be right bumber.
    // int            match[nmatch];

    int rc = cre2_set_match(rep->rex_set, text, text_len,
                    match, nmatch);
    // if (rc == 0) return 0;

    // for (int i=0; i<rc; i++) {
    //     LOG_INFO ("re_policy_match %d patterns  --> %d: %s",
    //             rc, match[i], rep->rules[match[i]].pattern);
    // }
    return rc;
}


static cre2_options_t* mk_cre2_opt()
{
    cre2_options_t*  opt;
    cre2_encoding_t  encoding;

    opt = cre2_opt_new();
    if (NULL == opt) {
        LOG_ERROR ("cre2_opt_new failed");
        return NULL;
    }

    // default UTF-8 == 1
    encoding = cre2_opt_encoding(opt);
    LOG_DEBUG ("cre2_opt_encoding: %d", encoding);
    cre2_opt_set_encoding(opt, CRE2_UTF8);
    LOG_DEBUG ("after set, cre2_opt_encoding: %d", cre2_opt_encoding(opt));

    cre2_opt_set_posix_syntax(opt, 1);

    // Restrict regexps to POSIX egrep syntax. Default is disabled.
    LOG_DEBUG ("cre2_opt_posix_syntax: %d", cre2_opt_posix_syntax(opt));
    LOG_DEBUG ("cre2_opt_longest_match: %d", cre2_opt_longest_match(opt));
    LOG_DEBUG ("cre2_opt_log_errors: %d", cre2_opt_log_errors(opt));
    LOG_DEBUG ("cre2_opt_literal: %d", cre2_opt_literal(opt));
    LOG_DEBUG ("cre2_opt_never_nl: %d", cre2_opt_never_nl(opt));
    LOG_DEBUG ("cre2_opt_dot_nl: %d", cre2_opt_dot_nl(opt));
    LOG_DEBUG ("cre2_opt_never_capture: %d", cre2_opt_never_capture(opt));
    cre2_opt_set_case_sensitive(opt, 0);
    LOG_DEBUG ("cre2_opt_case_sensitive: %d", cre2_opt_case_sensitive(opt));
    LOG_DEBUG ("cre2_opt_max_mem: %zu", cre2_opt_max_mem(opt));

    // Allow Perl’s \d, \s, \w, \D, \S, \W. Default is disabled.
    LOG_DEBUG ("cre2_opt_perl_classes: %d", cre2_opt_perl_classes(opt));

    // Allow Perl’s \b, \B (word boundary and not). Default is disabled.
    LOG_DEBUG ("cre2_opt_word_boundary: %d", cre2_opt_word_boundary(opt));

    // The patterns ^ and $ only match at the beginning and end of the text. Default is disabled.
    LOG_DEBUG ("cre2_opt_one_line: %d", cre2_opt_one_line(opt));

    // disable log syntax and execution errors to stderr
    cre2_opt_set_log_errors(opt, 0);

    return opt;
}

// load regex from file to lines array.
// TODO check reg wheter is valid.
UT_array* re_policy_load(const char* filename)
{
    UT_array*   lines = NULL;
    FILE*       fp = NULL;
    struct stat sb;
    char*       line = NULL;
    cre2_options_t* rex_opt = NULL;
    cre2_regexp_t* rex = NULL;

    LOG_INFO ("load regex policy from file '%s'", filename);

    if (stat(filename, &sb) == -1) {
        LOG_ERROR ("stat regex policy '%s' failed: %d, %s",
                    filename, errno, strerror(errno));
        return NULL;
    }

    if (NULL == (fp = fopen(filename, "r"))) {
        LOG_ERROR ("open regex policy '%s' failed: %d, %s",
                    filename, errno, strerror(errno));
        return NULL;
    }

    if (NULL == (line = malloc(sb.st_size))) {
        LOG_ERROR ("malloc line to read regex policy failed");
        return NULL;
    }

    if (NULL == (rex_opt = mk_cre2_opt())) {
        LOG_ERROR ("cre2_opt_new failed");
        goto err;
    }

    utarray_new(lines, &ut_str_icd);
    while (!feof(fp)) {
        int re = fscanf(fp, "%[^\n] ", line);
        if (re == EOF) break;
        LOG_DEBUG ("regex regex line %d: %s", strlen(line), line);

        // valid line is good regex string
        rex = cre2_new(line, strlen(line), rex_opt);
        if (rex) {
            if (!cre2_error_code(rex)) {
                utarray_push_back(lines, &line);
            } else {
                LOG_ERROR ("cre2_new failed: %d, %s",
                    cre2_error_code(rex), cre2_error_string(rex));
            }
            cre2_delete(rex);
            rex = NULL;
        } else {
            /* rex memory allocation error */
            LOG_ERROR ("cre2_new malloc failed");
        }
    }
    // while (fgets(line, sizeof(line), fp)) {
    //     LOG_DEBUG ("regex regex line %d: %s", strlen(line), line);
    // }

    cre2_opt_delete(rex_opt);
    free(line);
    fclose(fp);

    return lines;

err:
    if (rex) cre2_delete(rex);
    if (rex_opt) cre2_opt_delete(rex_opt);
    if (line) free(line);
    if (fp) fclose(fp);
    return NULL;
}

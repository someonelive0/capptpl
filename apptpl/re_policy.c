#include "re_policy.h"

#include "logger.h"


static cre2_options_t* mk_cre2_opt();


// create re_policy from policy filename.
int re_policy_create(struct re_policy* rep, const char* filename)
{
    cre2_set *        rex_set = NULL;
    cre2_options_t *  rex_opt = NULL;
    const char *      patterns[] = { "select", "insert", "update", NULL };
    char error[128];
    size_t errlen = sizeof(error);

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
        LOG_ERROR ("cre2_set_new failed: %d, %s\n",
                   cre2_error_code(rex_set), cre2_error_string(rex_set));
        goto err;
    }

    LOG_INFO ("read regex policy from file '%s'", filename);
    for (int i=0; patterns[i] != NULL ; i++) {
        if (-1 == cre2_set_add(rex_set, patterns[i], strlen(patterns[i]), error, errlen)) {
            LOG_ERROR ("cre2_set_add '%s' failed: %s\n", patterns[i], error);
            continue;
        }
        LOG_DEBUG ("cre2_set_add '%s'\n", patterns[i]);
    }
    if (!cre2_set_compile(rex_set)) {
        LOG_ERROR ("cre2_set_compile failed: %d, %s\n",
                   cre2_error_code(rex_set), cre2_error_string(rex_set));
        goto err;
    }

    rep->rex_opt = rex_opt;
    rep->rex_set = rex_set;
    return 0;

err:
    if (rex_set) cre2_set_delete(rex_set);
    if (rex_opt) cre2_opt_delete(rex_opt);
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
    return 0;
}

void re_policy_dump(struct re_policy* rep)
{
    LOG_INFO ("rule num: %d", rep->rule_num);
}

void re_policy_match(struct re_policy* rep, char* text, size_t text_len)
{
    int            nmatch   = 3; // we has 3 regex strings.
    int            match[nmatch];

    int rc = cre2_set_match(rep->rex_set, text, text_len,
                    match, nmatch);
    if (rc == 0) return;

    for (int i=0; i<rc; i++) {
        LOG_INFO ("re_policy_match : %d  --> %d", rc, match[i]);
    }
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
    LOG_DEBUG ("cre2_opt_encoding: %d\n", encoding);
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

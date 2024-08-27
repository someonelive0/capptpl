#ifndef RE_POLICY_H
#define RE_POLICY_H

#include <stddef.h>

#include "utarray.h"
#include "cre2.h"


// index begin with 0, for matched call back.
// pattern is regex partern.
struct re_rule {
    int    index;
    char*  pattern;
};

struct re_policy {
    int              rule_num;    // number of regexs.
    uint64_t         match_count; // on matched to callback
    cre2_options_t*  rex_opt;
    cre2_set*        rex_set;

    struct re_rule*  rules;       // rules array, has rule_num elements
};


// create re_policy from policy filename.
int re_policy_create(struct re_policy* rep, const char* filename);
int re_policy_destroy(struct re_policy* rep);
void re_policy_dump(struct re_policy* rep);

void re_policy_match(struct re_policy* rep, char* text, size_t text_len);

// load regex from file.
UT_array* re_policy_load(const char* filename);

#endif // RE_POLICY_H

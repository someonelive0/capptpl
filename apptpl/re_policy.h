#ifndef RE_POLICY_H
#define RE_POLICY_H

#include <stddef.h>

#include "cre2.h"


struct re_policy {
    int              rule_num;  // number of regexs.
    cre2_options_t*  rex_opt;
    cre2_set*        rex_set;

    // on matched to callback
    uint64_t         match_count;
};

// create re_policy from policy filename.
int re_policy_create(struct re_policy* rep, const char* filename);
int re_policy_destroy(struct re_policy* rep);
void re_policy_dump(struct re_policy* rep);

void re_policy_match(struct re_policy* rep, char* text, size_t text_len);


#endif // RE_POLICY_H

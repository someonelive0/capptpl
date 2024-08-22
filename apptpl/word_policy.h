#ifndef WORD_POLICY_H
#define WORD_POLICY_H

#include "msutil.h"
#include "acism.h"


struct word_policy {
    int     npatts;  // number of words.
    MEMBUF* patt;    // lines of policy file content.
    MEMREF* pattv;   // split lines to words.
    ACISM*  psp;     // acism object.

    // on matched to callback
    int (*match_cb)(int strnum, int textpos, MEMREF const *pattv);
    uint64_t match_count;
};

// create word_policy from policy filename.
int word_policy_create(struct word_policy* wordp, const char* filename);
int word_policy_destroy(struct word_policy* wordp);
void word_policy_dump(struct word_policy* wordp);

// static int on_match(int strnum, int textpos, MEMREF const *pattv);
void word_policy_match(struct word_policy* wordp, MEMREF text);


#endif // WORD_POLICY_H

#include "word_policy.h"

#include "logger.h"
#include "load_config.h"


int word_policy_create(struct word_policy* wordp, const char* filename)
{
    if (wordp->psp) {
        LOG_ERROR ("word_policy had created");
        return -1;
    }

    if (0 != copy_file_from_tpl(filename)) {
        LOG_ERROR ("word_policy '%s' or '%s.tpl' not existed", filename, filename);
        return -1;
    }

    MEMBUF patt = chomp(slurp(filename));
    if (!patt.ptr) {
        LOG_ERROR ("cannot read word policy file '%s'", filename);
        return -1;
    }

    int npatts;
    MEMREF *pattv = refsplit(patt.ptr, '\n', &npatts);

    double t = tick();
    ACISM *psp = acism_create(pattv, npatts);
    t = tick() - t;
    if (NULL == psp) {
        LOG_ERROR ("word_policy create failed");
        buffree(patt);
        free(pattv);
        return -1;
    }

    LOG_INFO ("word_policy (pattv[%d]) compiled, in %.3f seconds", npatts, t);

    memcpy(&wordp->patt, &patt, sizeof(MEMBUF));
    wordp->pattv = pattv;
    wordp->psp = psp;
    wordp->npatts = npatts;

    return 0;
}

int word_policy_destroy(struct word_policy* wordp)
{
    if (wordp->psp) {
        buffree(wordp->patt);
        free(wordp->pattv);
        acism_destroy(wordp->psp);
        wordp->pattv = NULL;
        wordp->psp = NULL;
    }

    return 0;
}


typedef struct {
    long long val;
    const char *name;
} PSSTAT;
extern PSSTAT psstat[];

#define UNUSED(x) (void)(x)
void word_policy_dump(struct word_policy* wordp)
{
    UNUSED(wordp);
    LOG_DEBUG ("acism dump");
    // on linux will cause coredump.
    // acism_dump(wordp->psp, PS_ALL, stderr, wordp->pattv);

    LOG_DEBUG ("acism stat");
    for (int i = 1; i < (int)psstat[0].val; ++i)
        if (psstat[i].val)
            fprintf(stderr, "%11llu %s\n", psstat[i].val, psstat[i].name);
}

void word_policy_match(struct word_policy* wordp, MEMREF text)
{
    int state = 0;
    (void)acism_more(wordp->psp,
                     text,
                     (ACISM_ACTION*)wordp->match_cb,
                     wordp->pattv,
                     &state);
}

/*
static int match_cb(int strnum, int textpos, MEMREF const *pattv)
{
    (void)strnum, (void)textpos, (void)pattv;
    match_count++;
    if (logger_isEnabled() <= LogLevel_DEBUG) {
        fprintf(stderr, "%9d %7d '%.*s'\n", textpos, strnum,
                (int)pattv[strnum].len, pattv[strnum].ptr);
    }
    return 0;
}
*/

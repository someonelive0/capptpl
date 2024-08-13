/*
 * gcc -O2 -W -Wall -I../lib/aho-corasick testac.o ../lib/aho-corasick/libacism.a -o testac
 * Usage: ./a.out words textfile
 */
#include "msutil.h"
#include <errno.h>
#include <fcntl.h> // open(2)
#include "acism.h"

typedef struct {
    long long val;
    const char *name;
} PSSTAT;
extern PSSTAT psstat[];
extern int pscand[];


static int actual = 0, details = 1;

static int on_match(int strnum, int textpos, MEMREF const *pattv)
{
    (void)strnum, (void)textpos, (void)pattv;
    ++actual;
    if (details) {
        fprintf(stderr, "%9d %7d '%.*s'\n", textpos, strnum,
                (int)pattv[strnum].len, pattv[strnum].ptr);
    }
    return 0;
}


// Usage: ./a.out words textfile
int main(int argc, char** argv)
{
    if (argc < 2 || argc > 4) {
        printf("pattern_file target_file [[-]expected]\ne.g. %s patts act.txt -5", argv[0]);
        exit(1);
    }

    MEMBUF patt = chomp(slurp(argv[1]));
    if (!patt.ptr) {
        printf("cannot read %s", argv[1]);
        exit(1);
    }
//重视
    int npatts;
    MEMREF *pattv = refsplit(patt.ptr, '\n', &npatts);

    double t = tick();
    ACISM *psp = acism_create(pattv, npatts);
    t = tick() - t;
    if (psp)
        printf("0 ----> acism_create(pattv[%d]) compiled, in %.3f secs\n", npatts, t);

    printf("1 ----> ac dump...\n");
    acism_dump(psp, PS_ALL, stderr, pattv);

    printf("2 ----> ac stat...\n"); // 重视
    for (int i = 1; i < (int)psstat[0].val; ++i)
        if (psstat[i].val)
            fprintf(stderr, "%11llu %s\n", psstat[i].val, psstat[i].name);

    FILE*	textfp = fopen(argv[2], "r");
    if (!textfp) die("cannot open %s", argv[2]);

    printf("3 ----> ac match...\n");
    static char buf[1024 * 1024];
    MEMREF		text = {buf, 0};
    int			state = 0;
    double		elapsed = 0, start = tick();
    while (0 < (text.len = fread(buf, sizeof*buf, sizeof buf, textfp))) {
        t = tick();
        (void)acism_more(psp, text, (ACISM_ACTION*)on_match, pattv, &state);
        elapsed += tick() - t;
        putc('.', stderr);
    }
    fclose(textfp);

    printf("\n4 ----> ac match finish...\n");
    printf("\ntext_file scanned in 1M blocks; read(s) took %.3f secs\n",
           tick() - start - elapsed);
    printf("%d matches found, in %.3f secs\n", actual, elapsed);

    buffree(patt);
    free(pattv);
    acism_destroy(psp);

    exit(0);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sds.h"

#include "hex.h"


int main()
{
    char buf[3];
    sds mystring;

    buf[0] = 'A';
    buf[1] = 'B';
    buf[2] = 'C';
    mystring = sdsnewlen(buf, 6);
    printf("'%s' has len %d\n", mystring, (int) sdslen(mystring));

    sds s1 = sdsempty();
    printf("empty s1 len is %d\n", (int) sdslen(s1));

    s1 = sdscat(s1, mystring);
    printf("sdscat s1 len is %d\n", (int) sdslen(s1));
    for (int i=0; i<10; i++)
        s1 = sdscatlen(s1, mystring, 6);
    printf("sdscat s1 len is %d\n", (int) sdslen(s1));

    printf("\nhexdump str with len %zu\n", sdslen(s1));
    dump_hex(s1, (int) sdslen(s1));

    // test string to hex
    char* strhex = (char*)malloc(sdslen(mystring)*2+1);
    to_hex(mystring, sdslen(mystring), strhex);
    strhex[sdslen(mystring) * 2]  = '\0';
    printf("hex %ld: '%s'\n", sdslen(mystring), strhex);

    char* str = (char*)malloc(strlen(strhex)/2+1);
    from_hex(strhex, strlen(strhex), str);
    str[strlen(strhex)/2] = '\0';
    printf("from_hex %ld: '%s'\n", sdslen(mystring), str);
    free(str);
    free(strhex);

    sdsfree(s1);
    sdsfree(mystring);

    exit(0);
}

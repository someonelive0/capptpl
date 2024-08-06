#include <stdlib.h>
#include <stdio.h>

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

    sdsfree(s1);
    sdsfree(mystring);

    exit(0);
}

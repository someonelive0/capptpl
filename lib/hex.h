#ifndef HEX_H
#define HEX_H

#include <stddef.h>


/* print mem to hex with 16 char in one line, looks like:
0000  41 42 43 80 09 40 41 42  43 80 09 40 41 42 43 80  |  ABC..@ABC..@ABC.
0016  09 40 41 42 43 80 09 40  41 42 43 80 09 40 41 42  |  .@ABC..@ABC..@AB
0032  43 80 09 40 41 42 43 80  09 40 41 42 43 80 09 40  |  C..@ABC..@ABC..@
0048  41 42 43 80 09 40 41 42  43 80 09 40 41 42 43 80  |  ABC..@ABC..@ABC.
0064  09 40                                             |  .@
*/
void dump_hex(const void* data, size_t size);

// bytes to hex string
// out should be sizeof(bin)*2 + 1
// out should add '\0' at the end by youself
void bin2hex(char *out, const char *bin, size_t binlen);

// hex string to bytes
// out should be sizeof(hex)/2+1
// out should add '\0' at the end by youself
size_t hex2bin(char *out, const char *hex, size_t hexlen);

#endif // HEX_H

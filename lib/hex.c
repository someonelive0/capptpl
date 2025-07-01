#include "hex.h"

#include <stdio.h>
#include <string.h>


/* print mem to hex with 16 char in one line, looks like:
0000  41 42 43 80 09 40 41 42  43 80 09 40 41 42 43 80  |  ABC..@ABC..@ABC.
0016  09 40 41 42 43 80 09 40  41 42 43 80 09 40 41 42  |  .@ABC..@ABC..@AB
0032  43 80 09 40 41 42 43 80  09 40 41 42 43 80 09 40  |  C..@ABC..@ABC..@
0048  41 42 43 80 09 40 41 42  43 80 09 40 41 42 43 80  |  ABC..@ABC..@ABC.
0064  09 40                                             |  .@
*/
void dump_hex(const void* data, size_t size)
{
    char ascii[17];
    size_t i, j, line = 0;
    ascii[16] = '\0';

    for (i = 0; i < size; ++i) {
        if (i % 16 == 0) {
            printf("%04zu  ", line);
            line += 16;
        }
        printf("%02X ", ((unsigned char*)data)[i]);

        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            printf(" ");
            if ((i+1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}



// ======================================================================
// bytes to hex string
// out should be sizeof(bin)*2 + 1
// out should add '\0' at the end by youself
void bin2hex(char *out, const char *bin, size_t binlen)
{
	size_t  i;

	for (i=0; i<binlen; i++) {
		out[i*2]   = "0123456789ABCDEF"[bin[i] >> 4];
		out[i*2+1] = "0123456789ABCDEF"[bin[i] & 0x0F];
	}
}

//  hex string to bytes
int hexchr2bin(const char hex, char *out)
{
	if (out == NULL)
		return 0;

	if (hex >= '0' && hex <= '9') {
		*out = hex - '0';
	} else if (hex >= 'A' && hex <= 'F') {
		*out = hex - 'A' + 10;
	} else if (hex >= 'a' && hex <= 'f') {
		*out = hex - 'a' + 10;
	} else {
		return 0;
	}

	return 1;
}

size_t hex2bin(char *out, const char *hex, size_t hexlen)
{
	size_t len;
	char   b1;
	char   b2;
	size_t i;

	if (hexlen % 2 != 0)
		return 0;
	len = hexlen/2;

	memset(out, 'A', len);
	for (i=0; i<len; i++) {
		if (!hexchr2bin(hex[i*2], &b1) || !hexchr2bin(hex[i*2+1], &b2)) {
			return 0;
		}
		out[i] = (b1 << 4) | b2;
	}

	return len;
}

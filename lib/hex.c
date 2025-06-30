#include "hex.h"

#include <stdio.h>


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

/*
这个其实很简单，追求速度的话，查表就好了
从0-16对应0-F即可：
*/
const char hex_table[] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

/* string to hex
需要确保输出缓冲区足够大，以存储转换后的16进制字符串。
输出字符串的大小是输入字符串长度的两倍，因为每个字符转换为16进制后占用两个字符的位置。
在实际应用中，可能需要考虑字符串的终止符\0，确保在转换后的字符串末尾添加终止符。
*/
/* all ok, the 3rd function is more quickly
void to_hex(char *s, int l, char *d)
{
    while(l--)
    {
        *(d+2*l+1) = hex_table[(*(s+l))&0x0f];
        *(d+2*l) = hex_table[(*(s+l))>>4];
    }
}

void to_hex(char *s, int l, char *d)
{
    while(l--)
    {
        *d = hex_table[*s >> 4];
        d++;
        *d = hex_table[*s & 0x0f];
        s++;
        d++;
    }
}
*/

void to_hex(char *s, int l, char *d)
{
    while(l--)
    {
        *(d++) = hex_table[*s >> 4];
        *(d++) = hex_table[*(s++) & 0x0f];
    }
}

/* hex to string
在实际应用中，可能需要考虑字符串的终止符\0，确保在转换后的字符串末尾添加终止符。
*/
/* all ok, the 3rd function is more quickly
void from_hex(char *s, int l, char *d)
{
    while(l--)
    {
        char* p = s+l;
        char* p2 = p-1;
        *(d+l/2) =
        ( (*p>'9'? *p+9 : *p) & 0x0f ) |
        ( (*p2>'9'? *p2+9 : *p2) << 4 );
        l--;
    }
}
*/
void from_hex(char *s, int l, char *d)
{
    while(l--)
    {
        *d = (*s>'9' ? *s+9 : *s) << 4;
        ++s;
        *d |= (*s>'9' ? *s+9 : *s) & 0x0F;
        ++s;
        ++d;
    }
}

/* because 内嵌“++”操作比单独写一行运行要快
 * but warning: operation on ‘s’ may be undefined [-Wsequence-point]
void from_hex(char *s, int l, char *d)
{
    while(l--)
    {
        *(d++) = ( (*s>'9' ? *(s++)+9 : *(s++)) << 4 )
        | ( (*s>'9' ? *(s++)+9 : *(s++)) & 0x0F );
    }
}
*/

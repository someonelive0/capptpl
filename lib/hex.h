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

/* string to hex
需要确保输出缓冲区足够大，以存储转换后的16进制字符串。
输出字符串的大小是输入字符串长度的两倍，因为每个字符转换为16进制后占用两个字符的位置。
在实际应用中，可能需要考虑字符串的终止符\0，确保在转换后的字符串末尾添加终止符。
*/
void to_hex(char *s, int l, char *d);

/* hex to string
在实际应用中，可能需要考虑字符串的终止符\0，确保在转换后的字符串末尾添加终止符。
*/
void from_hex(char *s, int l, char *d);

#endif // HEX_H

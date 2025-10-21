/*
只有符号的数才会发生溢出
对于signed整型的溢出，C的规范定义是“undefined behavior”，也就是说，编译器爱怎么实现就怎么实现。对于大多数编译器来说，仍然是回绕。

无符号数会回绕（常绕过一些判断语句）
对于unsigned整型溢出，C的规范是有定义的——“溢出后的数会以2^(8*sizeof(type))作模运算”，也就是说，如果一个unsigned char（1字符，8bits）溢出了，会把溢出的值与256求模。

  gcc testint.c && ./a.exe

*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // INT_MAX, INT_MIN, UINT_MAX

#include "nanoid.h"


void test_int();
void test_nanoid();

int main()
{
  test_int();
  test_nanoid();
}

void test_int() {
    puts("\ntest_nanid\n");

    // "无符号数会回绕 上面的代码会输出：0 （因为0xff + 1是256，与2^8求模后就是0
    unsigned char x = 0xff;
    printf("unsigned char 0xff + 1 = %d\n", ++x);

    // 第一种情况——有符合号溢出举个例子：
    int i;
    i = INT_MAX; // 2 147 483 647
    i++;
    printf("INT_MAX (2 147 483 647)+ 1 = i  = %d\n", i); // i = -2 147 483 648

    // 第二种情况——无符号回绕举个列子：
    unsigned int ui;
    ui = UINT_MAX; // 在 x86-32 上为 4 294 967 295
    ui++;
    printf("UINT_MAX (4 294 967 295) + 1 = ui = %u\n", ui); // ui = 0

    ui = 0;
    ui--;
    printf("0 - 1 = ui = %u\n", ui); // 在 x86-32 上，ui = 4 294 967 295

    // 第三种情况——高位截断截断举俩例子：
    // 加法截断：
    // 0xffffffff + 0x00000001
    // = 0x0000000100000000 (long long)
    // = 0x00000000 (long)

    // 乘法截断：
    // 0x00123456 * 0x00654321
    // = 0x000007336BF94116 (long long)
    // = 0x6BF94116 (long)

    // 漏洞多发函数
    // 1.*memcpy(void *dest, const void *src, size_t n)函数
    // 2.*strncpy(char *dest,const char *scr, size_t n)函数
    // ps说明：其中参数n，是size_t类型，size_t是一个无符号整型的类型
}

void test_nanoid() {
  puts("\ntest_nanid\n");
  int idlen = 10; // NANOIDLEN;

  for (int i=0; i<10; i++) {
    char *id = calloc(idlen + 1, 1);

    if (!id || nanoidgen2(id, idlen))
      return;

    puts(id);
    free(id);
  }
}

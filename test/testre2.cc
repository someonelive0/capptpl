/*
 * g++ -W -Wall -O2 -std=c++14 testre2.cc -lre2 -o testre2
 * g++ -W -Wall -O2 -std=c++14 -I/usr/local/re2-2024-03-01/include -I/usr/local/abseil-cpp-20240116.1/include -L/usr/local/re2-2024-03-01/lib testre2.cc -lre2 -o testre2
 */
#include <re2/re2.h>
#include <string>


int main() {
    RE2 re("hello, ([a-z]+)");
    std::string text = "hello, world";
    std::string match;

    if (RE2::FullMatch(text, re, &match)) {
        printf("Matched -%s- : %s\n", text.c_str(), match.c_str());
    } else {
        printf("Not match\n");
    }

    return 0;
}

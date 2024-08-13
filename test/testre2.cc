/*
 * g++ testre2.cc -lre2
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
        printf("No match\n", match);
    }

    return 0;
}

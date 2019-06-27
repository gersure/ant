#include <iostream>
#include "ant/sstring.hh"
#include "ant/print.hh"

int main() {
    ant::sstring sstr("init");
    sstr.append(" append", 7);
    ant::sstring sstr_find(&sstr[sstr.find("append")]);
    ant::print("sstr :%s\n", sstr);
    ant::print("sstr_find :%s\n", sstr_find);
    fmt::print("test {}\n", "hello world!");
    return 0;
}
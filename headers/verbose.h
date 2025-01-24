#ifndef VERBOSE_CLASS_H
#define VERBOSE_CLASS_H

#include <iostream>
#include <string>

class VerboseClass {
    bool verbose;
    std::ostream* os;
public:
    VerboseClass(bool verbose, std::ostream* os = &(std::cout));
    ~VerboseClass();
    bool get_verbose() const;
    bool print(std::string s) const;
};

#endif  // VERBOSE_CLASS_H

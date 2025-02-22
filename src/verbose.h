#ifndef VERBOSE_CLASS_H
#define VERBOSE_CLASS_H

#include <iostream>
#include <string>

class VerboseClass {
    bool verbose;
    std::ostream* os;
public:
    VerboseClass(bool verbose, std::ostream* os = &(std::cout)) {
        this->verbose = verbose;
        this->os = os;
    }
    ~VerboseClass() {}
    bool get_verbose() const {
        return this->verbose;
    }
    bool print(std::string s) const {
        if (get_verbose())
            *(os)<< s;
        return get_verbose();
    }
};

#endif  // VERBOSE_CLASS_H

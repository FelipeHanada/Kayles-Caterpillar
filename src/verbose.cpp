#include "verbose.h"

VerboseClass::VerboseClass(bool verbose, std::ostream *os) {
    this->verbose = verbose;
    this->os = os;
}

VerboseClass::~VerboseClass() {}

bool VerboseClass::get_verbose() {
    return this->verbose;
}

bool VerboseClass::print(std::string s) {
    if (get_verbose())
        *(os)<< s;
    return get_verbose();
}

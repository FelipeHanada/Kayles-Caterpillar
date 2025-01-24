#include "verbose.h"

VerboseClass::VerboseClass(bool verbose, std::ostream *os) {
    this->verbose = verbose;
    this->os = os;
}

VerboseClass::~VerboseClass() {}

bool VerboseClass::get_verbose() const {
    return this->verbose;
}

bool VerboseClass::print(std::string s) const {
    if (get_verbose())
        *(os)<< s;
    return get_verbose();
}

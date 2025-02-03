#include "kayles_caterpillar_mem.h"
#include <algorithm>

OrderedCaterpillar::OrderedCaterpillar(int n) : Caterpillar(n) {}

OrderedCaterpillar::OrderedCaterpillar(std::vector<int> x)
: Caterpillar(x) {
    int inversions = 0;
    for (int i = 0; i < x.size(); i++) {
        for (int j = i+1; j < x.size(); j++) {
            if (x[i] > x[j]) {
                inversions++;
            }
        }
    }

    int max_inversions = (x.size() * (x.size()-1)) / 2;

    if (inversions > max_inversions / 2) {
        std::reverse(this->x.begin(), this->x.end());
    }
}

Caterpillar* OrderedCaterpillarFactory::create(int n) {
    return new OrderedCaterpillar(n);
}

Caterpillar* OrderedCaterpillarFactory::create(std::vector<int> x) {
    return new OrderedCaterpillar(x);
}

MemoizedCaterpillarNimCalculator::MemoizedCaterpillarNimCalculator(AbstractCaterpillarFactory *factory)
: CaterpillarNimCalculator(factory) {
    this->nim_map = std::map<const std::vector<int>, unsigned int>();

    // fix memoization
}

MemoizedCaterpillarNimCalculator::MemoizedCaterpillarNimCalculator()
: MemoizedCaterpillarNimCalculator(new OrderedCaterpillarFactory()) {}

unsigned int MemoizedCaterpillarNimCalculator::calculate_nim(const Caterpillar *c, const VerboseClass &verb) {
    // Caterpillar *rc = this->factory->create(c->get_x());

    if (this->nim_map.find(c->get_x()) != this->nim_map.end()) {
        return this->nim_map[c->get_x()];
    }

    unsigned int nim = CaterpillarNimCalculator::calculate_nim(c, verb);
    this->nim_map[c->get_x()] = nim;

    // delete rc;
    return nim;
}


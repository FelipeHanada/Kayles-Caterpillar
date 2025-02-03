#ifndef KAYLES_CATERPILLAR_MEM_H
#define KAYLES_CATERPILLAR_MEM_H

#include "kayles_caterpillar.h"
#include <map>
#include <algorithm>


class OrderedCaterpillar : public Caterpillar {
public:
    OrderedCaterpillar(int n);
    OrderedCaterpillar(std::vector<int> x);
};

class OrderedCaterpillarFactory : public AbstractCaterpillarFactory {
public:
    Caterpillar* create(int n) override;
    Caterpillar* create(std::vector<int> x) override;
};

class MemoizedCaterpillarNimCalculator : public CaterpillarNimCalculator {
protected:
    std::map<const std::vector<int>, unsigned int> nim_map;
public:
    MemoizedCaterpillarNimCalculator(AbstractCaterpillarFactory *factory);
    MemoizedCaterpillarNimCalculator();

    unsigned int calculate_nim(const Caterpillar *c, const VerboseClass &verb = VerboseClass(false)) override;
};

#endif // KAYLES_CATERPILLAR_MEM_H

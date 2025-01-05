#ifndef KAYLES_CATERPILLAR_H
#define KAYLES_CATERPILLAR_H

#include <iostream>
#include <vector>
#include <set>
#include <math.h>

class Caterpillar {
protected:
    std::vector<int> x;
public:
    Caterpillar(int n);
    Caterpillar(std::vector<int> x);
    virtual ~Caterpillar();

    size_t size() const;
    const std::vector<int>& get_x() const;
    void print() const;
};

class AbstractCaterpillarFactory {
public:
    virtual Caterpillar* create(int n) = 0;
    virtual Caterpillar* create(std::vector<int> x) = 0;
};

class CaterpillarFactory : public AbstractCaterpillarFactory {
public:
    Caterpillar* create(int n) override;
    Caterpillar* create(std::vector<int> x) override;
};

class CaterpillarNimCalculator {
protected:
    AbstractCaterpillarFactory *factory;
public:
    CaterpillarNimCalculator();
    CaterpillarNimCalculator(AbstractCaterpillarFactory *factory);
    virtual ~CaterpillarNimCalculator();

    unsigned int calculate_play_nim(const Caterpillar* c, int i, bool p);
    virtual unsigned int calculate_nim(const Caterpillar *c);
};

#endif // KAYLES_CATERPILLAR_H

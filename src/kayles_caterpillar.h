#ifndef KAYLES_CATERPILLAR_H
#define KAYLES_CATERPILLAR_H

#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <math.h>
#include <memory>

class Caterpillar {
protected:
    const std::vector<unsigned int> x;
public:
    Caterpillar(unsigned int n)
        : x(std::vector<unsigned int>(n, 0)) {}
    Caterpillar(std::vector<unsigned int> x)
        : x(x) {}
    Caterpillar(const Caterpillar& c)
        : x(c.get_x()) {}
    virtual ~Caterpillar() {}
    size_t size() const {
        return x.size();
    }
    const std::vector<unsigned int>& get_x() const {
        return x;
    }
    std::string str() const {
        std::stringstream ss;
        ss << "(";
        for (size_t i = 0; i < x.size(); ++i)
            ss << x[i] << " ";
        ss << ")";
        return ss.str();
    }

    using const_iterator = std::vector<unsigned int>::const_iterator;
    const_iterator begin() const {
        return x.begin();
    }
    const_iterator end() const {
        return x.end();
    }
};

class AbstractCaterpillarFactory {
public:
    virtual std::unique_ptr<Caterpillar> create(unsigned int n) = 0;
    virtual std::unique_ptr<Caterpillar> create(std::vector<unsigned int> x) = 0;
};

class CaterpillarFactory : public AbstractCaterpillarFactory {
public:
    std::unique_ptr<Caterpillar> create(unsigned int n) override {
        return std::make_unique<Caterpillar>(n);
    }
    std::unique_ptr<Caterpillar> create(std::vector<unsigned int> x) override {
        return std::make_unique<Caterpillar>(x);
    }
};

class CaterpillarNimCalculator {
protected:
    std::unique_ptr<AbstractCaterpillarFactory> factory;
public:
    CaterpillarNimCalculator(std::unique_ptr<AbstractCaterpillarFactory> factory)
        : factory(std::move(factory)) {}

    CaterpillarNimCalculator()
        : CaterpillarNimCalculator(std::make_unique<CaterpillarFactory>()) {}

    unsigned int calculate_play_nim(const Caterpillar& c, int i, bool leaf) {
        /*
        Calcula o nimero da posicao apos uma jogada
        c: Caterpillar jogado
        i: vértice jogado
        p: no vértice solto
            (se a jogada não for possível, será jogado no caminho)
        */
        
        if (c.size() < 3)
            return 0;
    
        const std::vector<unsigned int> &x = c.get_x();
        if (x[i] == 0) leaf = false;
    
        if (i == 0) {
            std::vector<unsigned int> x1(x.begin() + 2, x.end());
            std::unique_ptr<Caterpillar> c1 = factory->create(x1);
            unsigned int nim = calculate_nim(*c1);
            if (x[1] % 2)
                nim ^= 1;
            return nim;
        }
    
        if (i == c.size() - 1) {
            std::vector<unsigned int> x1(x.begin(), x.end() - 2);
            std::unique_ptr<Caterpillar> c1 = factory->create(x1);
            unsigned int nim = calculate_nim(*c1);
            if (x.rbegin()[1] % 2)
                nim ^= 1;
            return nim;
        }
    
        if (i == 1) {
            if (c.size() == 3)
                return 0;
            
            std::vector<unsigned int> x1(x.begin() + 3, x.end());
            std::unique_ptr<Caterpillar> c1 = factory->create(x1);
            unsigned int nim = calculate_nim(*c1);
            if (x[0] % 2)
                nim ^= 1;
            if (x[2] % 2)
                nim ^= 1;
            return nim;
        }
        
        if (i == c.size() - 2) {
            std::vector<unsigned int> x1(x.begin(), x.end() - 3);
            std::unique_ptr<Caterpillar> c1 = factory->create(x1);
            unsigned int nim = calculate_nim(*c1);
            if (x.back() % 2)
                nim ^= 1;
            if (x.rbegin()[2] % 2)
                nim ^= 1;
            return nim;
        }
    
        if (leaf) {
            std::vector<unsigned int> x1(x.begin(), x.begin() + i);
            std::unique_ptr<Caterpillar> c1 = factory->create(x1);
    
            std::vector<unsigned int> x2(x.begin() + (i + 1), x.end());
            std::unique_ptr<Caterpillar> c2 = factory->create(x2);
    
            unsigned int nim = calculate_nim(*c1) ^ calculate_nim(*c2);
            
            if (x[i] % 2 == 0)
                nim ^= 1;
            return nim;
        } else {
            std::vector<unsigned int> x1(x.begin(), x.begin() + (i - 1));
            std::unique_ptr<Caterpillar>c1 = factory->create(x1);
    
            std::vector<unsigned int> x2(x.begin() + (i + 2), x.end());
            std::unique_ptr<Caterpillar>c2 = factory->create(x2);
    
            unsigned int nim = calculate_nim(*c1) ^ calculate_nim(*c2);
    
            if (x[i - 1] % 2)
                nim ^= 1;
            if (x[i + 1] % 2)
                nim ^= 1;
            return nim;
        }
    }
    std::set<unsigned int> get_mex_set(const Caterpillar& c) {
        const std::vector<unsigned int> &x = c.get_x();
        
        std::set<unsigned int> s;
        for (int i=0; i<c.size(); i++) {
            s.emplace(calculate_play_nim(c, i, false));
            if (x[i])
                s.emplace(calculate_play_nim(c, i, true));
        }
    
        return s;
    }
    virtual unsigned int calculate_nim(const Caterpillar& c) {
        if (c.size() == 0) return 0;

        std::set<unsigned int> s = get_mex_set(c);
        unsigned int nim = 0;
        for (auto it = s.begin(); it != s.end() && *it == nim; it++)
            nim++;
    
        return nim;
    }
};

#endif // KAYLES_CATERPILLAR_H

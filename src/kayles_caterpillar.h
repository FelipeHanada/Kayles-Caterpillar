#ifndef KAYLES_CATERPILLAR_H
#define KAYLES_CATERPILLAR_H

#include "verbose.h"
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <math.h>

class Caterpillar {
protected:
    std::vector<unsigned int> x;
public:
    Caterpillar(unsigned int n) {
        this->x = std::vector<unsigned int>(n, 0);
    }
    Caterpillar(std::vector<unsigned int> x) {
        if (x[0] != 0) {
            x[0] -= 1;
            x.insert(x.begin(), 0);
        }
    
        if (x[x.size() - 1] != 0) {
            x[x.size() - 1] -= 1;
            x.push_back(0);
        }
    
        this->x = x;
    }
    Caterpillar(const Caterpillar *c) {
        this->x = c->get_x();
    }
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
};

class AbstractCaterpillarFactory {
public:
    virtual Caterpillar* create(unsigned int n) = 0;
    virtual Caterpillar* create(std::vector<unsigned int> x) = 0;
};

class CaterpillarFactory : public AbstractCaterpillarFactory {
public:
    Caterpillar* create(unsigned int n) override {
        return new Caterpillar(n);
    }
    Caterpillar* create(std::vector<unsigned int> x) override {
        return new Caterpillar(x);
    }
};

class CaterpillarNimCalculator {
protected:
    AbstractCaterpillarFactory *factory;
public:
    CaterpillarNimCalculator(AbstractCaterpillarFactory *factory) {
        this->factory = factory;
    }
    CaterpillarNimCalculator()
    : CaterpillarNimCalculator(new CaterpillarFactory()) {
    }
    virtual ~CaterpillarNimCalculator() {
        delete factory;
    }

    unsigned int calculate_play_nim(const Caterpillar* c, int i, bool leaf) {
        /*
        Calcula o nimero da posicao apos uma jogada
        c: Caterpillar jogado
        i: vértice jogado
        p: no vértice solto
            (se a jogada não for possível, será jogado no caminho)
        */
        
        if (c->size() < 3)
            return 0;
    
        const std::vector<unsigned int> &x = c->get_x();
        if (x[i] == 0) leaf = false;
    
        if (i == 0) {
            std::vector<unsigned int> x1(x.begin() + 2, x.end());
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x[1] % 2)
                nim ^= 1;
            return nim;
        }
    
        if (i == c->size() - 1) {
            std::vector<unsigned int> x1(x.begin(), x.end() - 2);
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x.rbegin()[1] % 2)
                nim ^= 1;
            return nim;
        }
    
        if (i == 1) {
            if (c->size() == 3)
                return 0;
            
            std::vector<unsigned int> x1(x.begin() + 3, x.end());
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x[2] % 2)
                nim ^= 1;
            return nim;
        }
        
        if (i == c->size() - 2) {
            std::vector<unsigned int> x1(x.begin(), x.end() - 3);
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x.rbegin()[2] % 2)
                nim ^= 1;
            return nim;
        }
    
        if (leaf) {
            std::vector<unsigned int> x1(x.begin(), x.begin() + i);
            Caterpillar *c1 = factory->create(x1);
    
            std::vector<unsigned int> x2(x.begin() + (i + 1), x.end());
            Caterpillar *c2 = factory->create(x2);
    
            unsigned int nim = calculate_nim(c1) ^ calculate_nim(c2);
            delete c1;
            delete c2;
            
            if (x[i] % 2 == 0)
                nim ^= 1;
            return nim;
        } else {
            std::vector<unsigned int> x1(x.begin(), x.begin() + (i - 1));
            Caterpillar *c1 = factory->create(x1);
    
            std::vector<unsigned int> x2(x.begin() + (i + 2), x.end());
            Caterpillar *c2 = factory->create(x2);
    
            unsigned int nim = calculate_nim(c1) ^ calculate_nim(c2);
            delete c1;
            delete c2;
    
            if (x[i - 1] % 2)
                nim ^= 1;
            if (x[i + 1] % 2)
                nim ^= 1;
            return nim;
        }
    }
    std::set<unsigned int> get_mex_set(const Caterpillar *c) {
        const std::vector<unsigned int> &x = c->get_x();
        
        std::set<unsigned int> s;
        s.emplace(calculate_play_nim(c, 0, true));
        s.emplace(calculate_play_nim(c, c->size() - 1, true));
        for (int i=1; i<c->size() - 1; i++) {
            s.emplace(calculate_play_nim(c, i, true));
            if (x[i])
                s.emplace(calculate_play_nim(c, i, false));
        }
    
        return s;
    }
    virtual unsigned int calculate_nim(const Caterpillar *c, const VerboseClass &verb = VerboseClass(false)) {
        if (c->size() == 0) return 0;
        if (c->size() < 3) return 1;
    
        verb.print("================================");
        verb.print("Calculating nim for ");
        verb.print(c->str());
        verb.print("\n");
    
        std::set<unsigned int> s = get_mex_set(c);
    
        verb.print("mex_set = {");
        //  calcula mex
        unsigned int nim = 0;
        for (auto it = s.begin(); it != s.end() && *it == nim; it++) {
            verb.print(std::to_string(*it) + " ");
            nim++;
        }
        verb.print("}");
    
        verb.print("nim = " + std::to_string(nim));
        verb.print("================================");
    
        return nim;
    }
};

#endif // KAYLES_CATERPILLAR_H

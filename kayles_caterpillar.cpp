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
    Caterpillar(int n) {
        this->x = std::vector<int>(n, 0);
    }

    Caterpillar(std::vector<int> x) {
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

    virtual ~Caterpillar() {}

    size_t size() const {
        return x.size();
    }

    const std::vector<int>& get_x() const {
        return x;
    }

    void print() const {
        std::cout << "(";
        for (size_t i = 0; i < x.size(); ++i)
            std::cout << x[i] << " ";
        std::cout << ")";
    }
};

class AbstractCaterpillarFactory {
public:
    virtual Caterpillar* create(int n) = 0;
    virtual Caterpillar* create(std::vector<int> x) = 0;
};

class CaterpillarFactory : public AbstractCaterpillarFactory {
public:
    Caterpillar* create(int n) override {
        return new Caterpillar(n);
    }

    Caterpillar* create(std::vector<int> x) override {
        return new Caterpillar(x);
    }
};

class CaterpillarNimCalculator {
protected:
    AbstractCaterpillarFactory *factory;
public:
    CaterpillarNimCalculator() {
        factory = new CaterpillarFactory();
    }

    CaterpillarNimCalculator(AbstractCaterpillarFactory *factory) {
        this->factory = factory;
    }

    virtual ~CaterpillarNimCalculator() {
        delete factory;
    }

    unsigned int calculate_play_nim(const Caterpillar* c, int i, bool p) {
        /*
        Calcula o nimero da posicao apos uma jogada
        c: Caterpillar jogado
        i: vértice jogado
        p: jogada no vértice caminho / no vértice solto
            (se a jogada não for possível, será jogado no caminho)
        */
        
        if (c->size() < 3)
            return 0;

        const std::vector<int> &x = c->get_x();

        if (i == 0) {
            std::vector<int> x1(x.begin() + 2, x.end());
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x[1] % 2)
                nim ^= 1;
            return nim;
        }

        if (i == c->size() - 1) {
            std::vector<int> x1(x.begin(), x.end() - 2);
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x.rbegin()[1] % 2)
                nim ^= 1;
            return nim;
        }

        if (x[i] == 0)
            p = true;

        if (i == 1) {
            if (p) {
                if (c->size() == 3)
                    return 0;
                
                std::vector<int> x1(x.begin() + 3, x.end());
                Caterpillar *c1 = factory->create(x1);
                unsigned int nim = calculate_nim(c1);
                delete c1;
                if (x[2] % 2)
                    nim ^= 1;
                return nim;
            }

            std::vector<int> x1(x.begin() + 2, x.end());
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x[i] % 2)
                nim ^= 1;
            return calculate_nim(c1);
        }

        if (i == c->size() - 2) {
            if (p) {
                std::vector<int> x1(x.begin(), x.end() - 3);
                Caterpillar *c1 = factory->create(x1);
                unsigned int nim = calculate_nim(c1);
                delete c1;
                if (x.rbegin()[2] % 2)
                    nim ^= 1;
                return nim;
            }

            std::vector<int> x1(x.begin(), x.end() - 2);
            Caterpillar *c1 = factory->create(x1);
            unsigned int nim = calculate_nim(c1);
            delete c1;
            if (x[i] % 2)
                nim ^= 1;
            return nim;
        }

        if (p) {
            std::vector<int> x1(x.begin(), x.begin() + (i - 1));
            Caterpillar *c1 = factory->create(x1);

            std::vector<int> x2(x.begin() + (i + 2), x.end());
            Caterpillar *c2 = factory->create(x2);

            unsigned int nim = calculate_nim(c1) ^ calculate_nim(c2);
            delete c1;
            delete c2;

            if (x[i - 1] % 2)
                nim ^= 1;
            if (x[i + 1] % 2)
                nim ^= 1;
            return nim;
        } else {
            std::vector<int> x1(x.begin(), x.begin() + i);
            Caterpillar *c1 = factory->create(x1);

            std::vector<int> x2(x.begin() + (i + 1), x.end());
            Caterpillar *c2 = factory->create(x2);

            unsigned int nim = calculate_nim(c1) ^ calculate_nim(c2);
            delete c1;
            delete c2;
            
            if (x[i] % 2 == 0)
                nim ^= 1;
            return nim;
        }
    }

    virtual unsigned int calculate_nim(const Caterpillar *c) {
        if (c->size() == 0) return 0;

        const std::vector<int> &x = c->get_x();

        std::set<unsigned int> s;
        s.emplace(calculate_play_nim(c, 0, true));
        s.emplace(calculate_play_nim(c, c->size() - 1, true));
        for (int i=1; i<c->size() - 1; i++) {
            s.emplace(calculate_play_nim(c, i, true));
            if (x[i - 1])
                s.emplace(calculate_play_nim(c, i, false));
        }

        //  calcula mex
        unsigned int nim = 0;
        for (auto it = s.begin(); it != s.end() && *it == nim; it++)
            nim++;

        return nim;
    }
};

#endif // KAYLES_CATERPILLAR_H

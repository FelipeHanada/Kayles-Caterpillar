#include "kayles_ncaterpillar.h"
#include "tests.h"
#include <vector>
#include <map>
#include <filesystem>
#include <algorithm>
#include <iostream>

using namespace std;
typedef NCaterpillar<3> cat_t;
typedef NCaterpillarNimCalculator<3> calc_t;
typedef NCaterpillarNimFileManager<3> filem_t;
typedef NCaterpillarNimFile<3> file_t;
typedef unsigned int uint;
#define CALC_FOLDER "data/test/"
#define CALC_FILE_PREFIX CALC_FOLDER "nim_file_"

vector<pair<pair<int, int>, vector<uint>>> caterpillars = {
    { { 0, 0 }, {} },
    { { 1, 0 }, { 0 } },
    { { 2, 0 }, { 0, 0} },
    { { 3, 0 }, { 0, 0, 0} },
    { { 4, 0 }, { 0, 0, 0, 0 } },

    { { 3, 1 }, { 0, 1, 0} },
    { { 4, 1 }, { 0, 1, 0, 0 } },
    { { 5, 1 }, { 0, 1, 0, 0, 0 } },
    { { 6, 1 }, { 0, 1, 0, 0, 0, 0 } },
    { { 7, 1 }, { 0, 1, 0, 0, 0, 0, 0 } },

    { { 3, 2 }, { 0, 2, 0} },
    { { 4, 2 }, { 0, 2, 0, 0 } },
    { { 5, 2 }, { 0, 2, 0, 0, 0 } },
    { { 6, 2 }, { 0, 2, 0, 0, 0, 0 } },
    { { 7, 2 }, { 0, 2, 0, 0, 0, 0, 0 } },

    { { 4, 3 }, { 0, 0, 1, 0 } },
    { { 5, 3 }, { 0, 0, 1, 0, 0 } },
    { { 6, 3 }, { 0, 0, 1, 0, 0, 0 } },
    { { 7, 3 }, { 0, 0, 1, 0, 0, 0, 0 } },
};
TEST(NcaterpillarConstructor) {
    for (auto test : caterpillars) {
        cat_t *c1 = new cat_t(test.first.first, test.first.second);
        cat_t *c2 = new cat_t(test.second);

        ASSERT(c1->get_x() == c2->get_x(), "error on test " << test.first.first << " " << test.first.second);

        delete c1;
        delete c2;
    }
}

vector<pair<cat_t, uint>> nims = {
    { cat_t(0, 0), 0 },
    { cat_t(1, 0), 1 },
    { cat_t(2, 0), 1 },
    { cat_t(3, 0), 2 },
    { cat_t(4, 0), 0 },

    { cat_t(3, 1), 1 },

    { cat_t((vector<uint>){0, 0}), 1 },
    { cat_t((vector<uint>){0, 0, 0}), 2 },
    { cat_t((vector<uint>){0, 2, 0}), 2 },
    { cat_t((vector<uint>){0, 1, 0, 0}), 3 },
    { cat_t((vector<uint>){0, 3, 0, 0}), 3 },
    { cat_t((vector<uint>){0, 0, 2, 0, 0}), 2 },
    { cat_t((vector<uint>){0, 3, 1, 1, 0}), 1 },
    { cat_t((vector<uint>){0, 3, 1, 2, 0, 0}), 0 },
};
TEST(CalculatorTest1) {
    for (auto test : nims) {
        calc_t *calc = new calc_t(CALC_FILE_PREFIX);
        uint nim = calc->calculate_nim(&test.first);
        ASSERT(nim == test.second, test.first.str());

        delete calc;
        std::filesystem::remove_all(CALC_FOLDER);
    }
}
TEST(CalculatorTest2) {
    calc_t *calc = new calc_t(CALC_FILE_PREFIX);
    for (auto test : nims) {
        uint nim = calc->calculate_nim(&test.first);
        ASSERT(nim == test.second, test.first.str());
    }
    delete calc;
    std::filesystem::remove_all(CALC_FOLDER);
}
TEST(CalculatorTest3){
    calc_t *calc = new calc_t(CALC_FILE_PREFIX);
    for (auto test : nims) {
        vector<uint> reverse_x = test.first.get_x();
        reverse(reverse_x.begin(), reverse_x.end());
        cat_t *c = new cat_t(reverse_x);

        uint nim = calc->calculate_nim(&test.first);
        uint nim2 = calc->calculate_nim(c);

        ASSERT(nim == nim2 && nim == test.second, c->str());
    }
    delete calc;
    std::filesystem::remove_all(CALC_FOLDER);
}
TEST(CalculatorTest4){
    calc_t *calc = new calc_t(CALC_FILE_PREFIX);
    filem_t *filem = calc->get_file_manager();
    for (auto test : nims) {
        file_t *file = filem->get_file(test.first.get_x_class(), true);

        if (file->get_n0() > test.first.get_x().size())
            continue;

        uint nim = calc->calculate_nim(&test.first);
        uint nim2 = file->read(file->hash(test.first.size()));

        ASSERT(nim == nim2 && nim == test.second, test.first.str());
    }
    delete calc;
    std::filesystem::remove_all(CALC_FOLDER);
}

int main() {
    run_all_tests();
    cout << "Tests completed" << endl;
    return 0;
}

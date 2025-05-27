#include "kayles_ncaterpillar.h"
#include "tests.h"
#include <vector>
#include <map>
#include <filesystem>
#include <algorithm>
#include <iostream>

using namespace std;
typedef unsigned int uint;
#define TEST_FOLDER ".data/test/"
#define TEST_FILE_PREFIX TEST_FOLDER "nim_file_"

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
        unique_ptr<NCaterpillar<3>> c1 = make_unique<NCaterpillar<3>>(test.first.first, test.first.second);
        unique_ptr<NCaterpillar<3>> c2 = make_unique<NCaterpillar<3>>(test.second);

        std::vector<unsigned int> reversed(c2->get_x().rbegin(), c2->get_x().rend());
        ASSERT((c1->get_x() == c2->get_x()) || (c1->get_x() == reversed), "error on test " << test.first.first << " " << test.first.second);
    }
}

vector<pair<NCaterpillar<3>, uint>> nims = {
    { NCaterpillar<3>(0, 0), 0 },
    { NCaterpillar<3>(1, 0), 1 },
    { NCaterpillar<3>(2, 0), 1 },
    { NCaterpillar<3>(3, 0), 2 },
    { NCaterpillar<3>(4, 0), 0 },

    { NCaterpillar<3>(3, 1), 1 },

    { NCaterpillar<3>((vector<uint>){0, 0}), 1 },
    { NCaterpillar<3>((vector<uint>){0, 0, 0}), 2 },
    { NCaterpillar<3>((vector<uint>){0, 2, 0}), 2 },
    { NCaterpillar<3>((vector<uint>){0, 1, 0, 0}), 3 },
};

TEST(CalculatorTest1) {
    for (auto test : nims) {
        filesystem::create_directories(TEST_FOLDER);

        {
            unique_ptr<NCaterpillarNimCalculator<3>> calc = make_unique<NCaterpillarNimCalculator<3>>(TEST_FILE_PREFIX);
            uint nim = calc->calculate_nim(test.first);
            ASSERT(nim == test.second, test.first.str());
        }

        filesystem::remove_all(TEST_FOLDER);
    }
}

TEST(CalculatorTest2) {
    filesystem::create_directories(TEST_FOLDER);
    {
        unique_ptr<NCaterpillarNimCalculator<3>> calc = make_unique<NCaterpillarNimCalculator<3>>(TEST_FILE_PREFIX);
        for (auto test : nims) {
            uint nim = calc->calculate_nim(test.first);
            ASSERT(nim == test.second, test.first.str());
        }
    }
    filesystem::remove_all(TEST_FOLDER);
}

TEST(CalculatorTest3){
    filesystem::create_directories(TEST_FOLDER);
    {
        unique_ptr<NCaterpillarNimCalculator<3>> calc = make_unique<NCaterpillarNimCalculator<3>>(TEST_FILE_PREFIX);
        for (auto test : nims) {
            vector<uint> reverse_x = test.first.get_x();
            reverse(reverse_x.begin(), reverse_x.end());
            
            unique_ptr<NCaterpillar<3>> c = make_unique<NCaterpillar<3>>(reverse_x);
    
            uint nim = calc->calculate_nim(test.first);
            uint nim2 = calc->calculate_nim(*c);
    
            ASSERT(nim == nim2 && nim == test.second, c->str());
        }
    }
    filesystem::remove_all(TEST_FOLDER);
}

TEST(CalculatorTest4){
    filesystem::create_directories(TEST_FOLDER);
    {
        unique_ptr<NCaterpillarNimCalculator<3>> calc = make_unique<NCaterpillarNimCalculator<3>>(TEST_FILE_PREFIX);
        NCaterpillarNimFileManager<3>& filem = calc->get_file_manager();
        for (auto test : nims) {
            NCaterpillarNimFile<3>& file = filem.get_file(test.first.get_x_class());
            filem.open_file(test.first.get_x_class());
    
            if (file.get_n0() > test.first.get_x().size())
                continue;
    
            uint nim = calc->calculate_nim(test.first);
            uint nim2 = file.read(file.hash(test.first.size()));
    
            ASSERT(nim == nim2 && nim == test.second, test.first.str());
        }
    }
    filesystem::remove_all(TEST_FOLDER);
}

int main() {
    run_all_tests();
    cout << "Tests completed" << endl;
    return 0;
}

#include <vector>
#include <functional>
#include <iostream>

#define TEST(name) \
    void name(); \
    bool name##_registered = register_test(name); \
    void name()

#define ASSERT(condition, message) \
    if (!(condition)) std::cerr << "Test FAILED in " << __FILE__ << " line " << __LINE__ << ": " << message << std::endl; \
    else std::cout << "Test passed in " << __FILE__ << " line " << __LINE__ << std::endl;

std::vector<std::function<void()>>& get_tests() {
    static std::vector<std::function<void()>> tests;
    return tests;
}

bool register_test(std::function<void()> test) {
    get_tests().push_back(test);
    return true;
}

void run_all_tests() {
    for (auto& test : get_tests()) {
        test();
    }
}

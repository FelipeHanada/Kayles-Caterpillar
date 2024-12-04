#include "kayles_caterpillar_nim_calculator.cpp"


int main() {
    CaterpillarNimCalculator calculator("data/caterpillar_nim_");

    // run_tests(calculator);
    calculate_by_time(calculator, 1, hours(10), minutes(15));

    return 0;
}

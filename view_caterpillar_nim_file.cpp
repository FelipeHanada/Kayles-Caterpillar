#include <iostream>
#include <fstream>
#include <vector>

int main() {
    std::ifstream inputFile("caterpillar_nim_1", std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    unsigned int number;
    unsigned int n = 0;
    std::cout << "Read numbers:" << std::endl;
    while (inputFile.read(reinterpret_cast<char*>(&number), sizeof(unsigned int))) {
        std::cout << number << ' ';
        n++;
    }
    std::cout << std::endl;
    std::cout << "Number of elements: " << n << std::endl;
    inputFile.close();


    return 0;
}
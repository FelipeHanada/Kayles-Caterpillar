#include <bits/stdc++.h>

using namespace std;


#define INPUTFILE "caterpillar_nim_1"
#define OUTPUTFILE "caterpillar_nim_1.txt"


int main() {
    fstream input_file;
    ofstream output_file;
    input_file.open(INPUTFILE, ios::in | ios::binary);
    output_file.open(OUTPUTFILE, ios::out | ios::trunc);

    unsigned int n;
    while (input_file.read((char*) &n, sizeof(unsigned int)))
        output_file << n << " ";
    output_file << endl;

    input_file.close();
    output_file.close();
}

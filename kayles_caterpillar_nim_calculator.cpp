#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <set>
#include <map>
#include <math.h>

using namespace std;
using namespace std::chrono;

typedef unsigned int uint;
#define DEFAULT_CACHE_SIZE (1 << 30)


class Caterpillar {
public:
    int n;
    vector<bool> x;

    Caterpillar(int n, uint x)
    : n(n), x(max(0, n - 2)) {
        int j = min(this->x.size(), sizeof(x) * 8);
        for (int i=0; i < j; i++)
            this->x[i] = x & (1 << i);
    }

    Caterpillar(int n, const vector<bool> &x)
    : n(n), x(max(0, n - 2)) {
        int j = min(n - 2, (int) x.size());
        for (int i=0; i < j; i++)
            this->x[i] = x[i];
    }

    uint get_x_class() const {
        uint x_class = 0;
        for (size_t i=0; i<this->x.size(); i++)
            x_class += (this->x[i]) ? (1 << i) : 0;
        return x_class;
    }

    static Caterpillar open_right(int n, vector<bool> &x) {
        // recebe um Caterpillar sem vértice normal "0" à direita
        // retorna ele com vértice 0 à direita
        // exemplo: 0(010101011101 -> 0(010101011100)0
        if (n == 1) return Caterpillar(1, 0);
        if (x.back()) {
            vector<bool> x1(x.begin(), x.end());
            x1.back() = false;
            return Caterpillar(n + 1, x1);
        }
        vector<bool> x1(x.begin(), x.end() - 1);
        return Caterpillar(n, x1);
    }

    static Caterpillar open_left(int n, vector<bool> &x) {
        // recebe um Caterpillar sem vértice normal "0" à esquerda
        // retorna ele com vértice 0 à esquerda
        // exemplo: 010101011101)0 -> 0(10101011101)0
        if (n == 1) return Caterpillar(1, 0);
        if (x.front()) {
            vector<bool> x1(x.begin(), x.end());
            x1.front() = false;
            return Caterpillar(n + 1, x1);
        }
        vector<bool> x1(x.begin() + 1, x.end());
        return Caterpillar(n, x1);
    }

    void print() const {
        cout << 0 << "(";
        for (bool xi : this->x)
            cout << (xi ? 1 : 0);
        cout << ")" << 0;
    }
};

class CaterpillarNimFile {
    string filename;
    fstream file;
    size_t file_size;
    uint x_class;
    size_t offset;
    size_t cache_size;
    vector<uint> cache;
public:
    CaterpillarNimFile(string filename, uint x_class, size_t cache_size = DEFAULT_CACHE_SIZE) {
        this->filename = filename;
        this->x_class = x_class;
        this->offset = (x_class == 0) ? 0 : 3 + log2(x_class);
        this->cache_size = cache_size;
        this->cache = vector<uint>(cache_size);

        filesystem::create_directories(filesystem::path(filename).parent_path());
        file.open(filename, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            file.clear();
            file.open(filename, ios::out | ios::binary);

            file.close();
            file.open(filename, ios::in | ios::out | ios::binary);
        }

        if (!this->file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            return;
        }

        this->file.seekg(0, ios::end);
        this->file_size = file.tellg();

        for (int i=0; i<cache_size && i<this->size(); i++)
            this->cache[i] = this->raw_read_pos(i);
    }

    ~CaterpillarNimFile() {
        this->file.close();
    }
    
    size_t size() {
        return this->file_size / sizeof(uint);
    }

    size_t get_pos(uint n) {
        if (n < offset)
            return 0;
        return n - offset;
    }

    int get_n(size_t pos) {
        return pos + offset;
    }

    uint raw_read_pos(size_t pos) {
        uint nim;
        this->file.seekg(pos * sizeof(uint));
        this->file.read((char*)&nim, sizeof(uint));
        return nim;
    }

    uint read_pos(size_t pos) {
        if (pos < this->cache_size)
            return this->cache[pos];

        return raw_read_pos(pos);
    }

    uint read_n(uint n) {
        return read_pos(get_pos(n));
    }

    uint is_calculated(uint n) {
        return get_pos(n) < this->size();
    }

    void write(uint nim) {
        if (this->size() < this->cache_size)
            this->cache[this->size()] = nim;

        this->file.seekp(0, ios::end);
        this->file.write((char*)&nim, sizeof(uint));
        this->file_size += sizeof(uint);
    }

    void write(uint n, uint nim) {
        size_t pos = get_pos(n);

        if (pos >= this->size())
            return write(nim);

        if (pos < this->cache_size)
            this->cache[n] = nim;

        this->file.seekp(pos * sizeof(uint));
        this->file.write((char*)&nim, sizeof(uint));
    }
};

class CaterpillarNimCalculator {
    vector<CaterpillarNimFile*> files;
    string file_prefix;
    size_t default_cache_size;
public:
    CaterpillarNimCalculator(string file_prefix, size_t default_cache_size = DEFAULT_CACHE_SIZE) {
        this->file_prefix = file_prefix;
        this->default_cache_size = default_cache_size;

        CaterpillarNimFile *file0 = this->get_file(0);
        file0->write(0, 0);
        file0->write(1, 1);
    }

    ~CaterpillarNimCalculator() {
        for (auto file : files)
            delete file;
    }

    CaterpillarNimFile* get_file(uint x) {
        for (uint i=files.size(); i <= x; i++)
            files.push_back(new CaterpillarNimFile(
                file_prefix + (to_string(i)),
                i,
                default_cache_size
            ));
        
        return files[x];
    }

    uint calculate_play_nim(const Caterpillar &c, int i, bool p) {
        /*
        c: Caterpillar jogado
        i: vértice jogado
        p: jogada no vértice caminho / no vértice solto
            (se a jogada não for possível, será jogado no caminho)
        */
        
        if (c.n == 1 || c.n == 2)
            return calculate_nim(Caterpillar(0, 0));

        if (i == 0) {
            vector<bool> x1(c.x.begin() + 1, c.x.end());
            Caterpillar c1 = Caterpillar::open_left(c.n - 2, x1);
            uint nim = calculate_nim(c1);
            if (c.x.front())
                nim ^= calculate_nim(Caterpillar(1, 0));
            return nim;
        }

        if (i == c.n - 1) {
            vector<bool> x1(c.x.begin(), c.x.end() - 1);
            Caterpillar c1 = Caterpillar::open_right(c.n - 2, x1);
            uint nim = calculate_nim(c1);
            if (c.x.back())
                nim ^= calculate_nim(Caterpillar(1, 0));
            return nim;
        }

        if (i == 1) {
            if (p || !c.x.front()) {
                vector<bool> x1(c.x.begin() + 2, c.x.end());
                Caterpillar c1 = Caterpillar::open_left(c.n - 3, x1);
                uint nim = calculate_nim(c1);
                if (c.x[1])
                    nim ^= calculate_nim(Caterpillar(1, 0));
                return nim;
            }

            vector<bool> x1(c.x.begin() + 1, c.x.end());
            Caterpillar c1 = Caterpillar::open_left(c.n - 2, x1);
            return calculate_nim(c1) ^ calculate_nim(Caterpillar(1, 0));
        }

        if (i == c.n - 2) {
            if (p || !c.x.back()) {
                vector<bool> x1(c.x.begin(), c.x.end() - 2);
                Caterpillar c1 = Caterpillar::open_right(c.n - 3, x1);
                uint nim = calculate_nim(c1);
                if (*(c.x.rbegin() + 1))
                    nim ^= calculate_nim(Caterpillar(1, 0));
                return nim;
            }

            vector<bool> x1(c.x.begin(), c.x.end() - 1);
            Caterpillar c1 = Caterpillar::open_right(c.n - 2, x1);
            return calculate_nim(c1) ^ calculate_nim(Caterpillar(1, 0));
        }

        i--;  // ajusta i para usar como índice de x
        if (p || !c.x[i]) {
            vector<bool> x1(c.x.begin(), c.x.begin() + (i - 1));
            Caterpillar c1 = Caterpillar::open_right(i, x1);
            vector<bool> x2(c.x.begin() + (i + 2), c.x.end());
            Caterpillar c2 = Caterpillar::open_left(c.n - (i + 3), x2);

            uint nim = calculate_nim(c1) ^ calculate_nim(c2);
            if (c.x[i + 1])
                nim ^= calculate_nim(Caterpillar(1, 0));
            if (c.x[i - 1])
                nim ^= calculate_nim(Caterpillar(1, 0));

            return nim;
        } else {
            vector<bool> x1(c.x.begin(), c.x.begin() + i);
            Caterpillar c1 = Caterpillar::open_right(i + 1, x1);
            vector<bool> x2(c.x.begin() + (i + 1), c.x.end());
            Caterpillar c2 = Caterpillar::open_left(c.n - (i + 2), x2);
            return calculate_nim(c1) ^ calculate_nim(c2);
        }
    }

    uint calculate_nim(const Caterpillar &c) {
        CaterpillarNimFile *file = get_file(c.get_x_class());
        if (file->is_calculated(c.n))
            return file->read_n(c.n);

        if (c.n > file->get_n(file->size()))
            calculate_nim(Caterpillar(c.n - 1, c.x));
        //  certifica-se que o caterpillar anterior já foi calculado
        //  para manter a sequencialidade do arquivo

        set<uint> s;

        s.emplace(calculate_play_nim(c, 0, true));
        s.emplace(calculate_play_nim(c, c.n - 1, true));

        for (int i=1; i<c.n - 1; i++) {
            s.emplace(calculate_play_nim(c, i, true));
            if (c.x[i - 1])
                s.emplace(calculate_play_nim(c, i, false));
        }

        //  calcula mex
        uint nim = 0;
        for (auto it = s.begin(); it != s.end() && *it == nim; it++)
            nim++;
        
        file->write(nim);
        return nim;
    }
};

void run_tests(CaterpillarNimCalculator &calculator) {
    cout << "CATERPILLAR TESTS" << endl;
    cout << "Testing caterpillar x_class converter" << endl;
    for (int i=0; i<10; i++) {
        Caterpillar c(100, i);
        cout << "\t";
        if (c.get_x_class() == i)
            cout << "Success";
        else
            cout << "Failed";
        cout << " - x=" << i << endl;
    }
    
    cout << "CALCULATOR TESTS" << endl;
    cout << "Testing files n-position converter" << endl;
    for (int i=0; i<3; i++) {
        CaterpillarNimFile *file = calculator.get_file(i);
        cout << "testing for x=" << i << endl;
        for (int j=0; j<10; j++) {
            cout << "\t";
            if (file->get_pos(file->get_n(j)) == j)
                cout << "Success";
            else
                cout << "Failed";
            cout << " - pos=" << j << endl;
        }
    }

    cout << "Testing calculator file writing" << endl;
    for (int i=0; i<3; i++) {
        CaterpillarNimFile *file = calculator.get_file(i);
        cout << "testing for x=" << i << endl;
        for (int j=0; j<10; j++) {
            int n = file->get_n(j);
            uint nim = calculator.calculate_nim(Caterpillar(n, i));
            cout << "\t";
            if (file->read_pos(j) == nim)
                cout << "Success";
            else
                cout << "Failed";
            cout << " - n=" << n << endl;
        }
    }

    cout << "Testing file nim getters" << endl;
    for (int i=0; i<3; i++) {
        CaterpillarNimFile *file = calculator.get_file(i);
        cout << "testing for x=" << i << endl;
        for (int j=0; j<10; j++) {
            int n = file->get_n(j);
            calculator.calculate_nim(Caterpillar(n, i));
            cout << "\t";
            if (file->read_n(n) == file->read_pos(j))
                cout << "Success";
            else
                cout << "Failed";
            cout << " - n=" << n << endl;
        }
    }

    cout << "Testing cache" << endl;
    for (int i=0; i<3; i++) {
        CaterpillarNimFile *file = calculator.get_file(i);
        cout << "testing for x=" << i << endl;
        for (int j=0; j<100; j++) {
            calculator.calculate_nim(Caterpillar(file->get_n(j), i));
            cout << "\t";
            if (file->read_pos(j) == file->raw_read_pos(j))
                cout << "Success";
            else
                cout << "Failed";
            cout << " - pos=" << j << endl;
        }
    }

    map<pair<int, vector<bool>>, pair<int, vector<bool>>> open_right_tests = {
        {{2, {0}}, {2, {}}},
        {{2, {1}}, {3, {0}}},
        {{13, {0,1,0,1,0,1,0,1,1,1,0,0}}, {13, {0,1,0,1,0,1,0,1,1,1,0}}},
        {{13, {0,1,0,1,0,1,0,1,1,1,0,1}}, {14, {0,1,0,1,0,1,0,1,1,1,0,0}}}
    };
    cout << "OPEN_RIGHT TESTS" << endl;
    for (auto test : open_right_tests) {
        auto input = test.first;
        auto expected = test.second;
        Caterpillar c = Caterpillar::open_right(input.first, input.second);
        Caterpillar expected_c(expected.first, expected.second);
        
        cout << "\t";
        if (c.n == expected.first && c.x == expected.second) {
            cout << "Success. ";
        } else {
            cout << "Failed. Expected: ";
            expected_c.print();
            cout << " ";
        }
        cout << "Found: ";
        c.print();
        cout << endl;
    }

    map<pair<int, vector<bool>>, pair<int, vector<bool>>> open_left_tests = {
        {{2, {0}}, {2, {}}},
        {{2, {1}}, {3, {0}}},
        {{13, {0,1,0,1,0,1,0,1,1,1,0,1}}, {13, {1,0,1,0,1,0,1,1,1,0,1}}},
        {{13, {1,1,0,1,0,1,0,1,1,1,0,1}}, {14, {0,1,0,1,0,1,0,1,1,1,0,1}}}
    };
    cout << "OPEN_LEFT TESTS" << endl;
    for (auto test : open_left_tests) {
        auto input = test.first;
        auto expected = test.second;
        Caterpillar c = Caterpillar::open_left(input.first, input.second);
        Caterpillar expected_c(expected.first, expected.second);

        cout << "\t";
        if (c.n == expected.first && c.x == expected.second) {
            cout << "Success. ";
        } else {
            cout << "Failed. Expected: ";
            expected_c.print();
            cout << " ";
        }
        cout << "Found: ";
        c.print();
        cout << endl;
    }

    vector<pair<Caterpillar, uint>> nim_tests = {
        {Caterpillar(0, 0), 0},
        {Caterpillar(1, 0), 1},
        {Caterpillar(4, 0), 0},
        {Caterpillar(3, 1), 1},
        {Caterpillar(4, 1), 3},
        {Caterpillar(5, 1), 0},
        {Caterpillar(6, 1), 0},
        {Caterpillar(7, 1), 1}
    };
    cout << "NIM TESTS" << endl;
    for (auto test : nim_tests) {
        uint nim = calculator.calculate_nim(test.first);
        
        cout << "\t";
        cout << "Calculating for n=" << test.first.n << " x=" << test.first.get_x_class() << " - ";
        if (nim == test.second)
            cout << "Success";
        else
            cout << "Failed. Expected: " << test.second << ", Found: " << nim;
        cout << endl;
    }
}

void calculate_by_n(
    CaterpillarNimCalculator &calculator,
    uint x_class,
    uint n_limit,
    const milliseconds &display_interval = minutes(1)
) {
    auto start = high_resolution_clock::now();
    auto next_display_time = start + display_interval;

    CaterpillarNimFile *file = calculator.get_file(x_class);

    int n = file->get_n(file->size());
    
    cout << "Starting calculations from n=" << n << endl;
    while (n < n_limit) {
        auto now = high_resolution_clock::now();
        if (now >= next_display_time) {
            auto elapsed = duration_cast<milliseconds>(now - start);
            cout << "Time elapsed: " << elapsed.count() << " millisecond(s). ";
            cout << "Last calculated n=" << n - 1 << endl;
            next_display_time += display_interval;
        }

        calculator.calculate_nim(Caterpillar(n, x_class));
        n++;
    }

    cout << "End of calculations. Last calculated n=" << n - 1 << endl;
}

void calculate_by_time(
    CaterpillarNimCalculator &calculator,
    uint x_class,
    const milliseconds &time_limit,
    const milliseconds &display_interval = minutes(1)
) {
    auto start = high_resolution_clock::now();
    auto next_display_time = start + display_interval;

    CaterpillarNimFile *file = calculator.get_file(x_class);

    int n = file->get_n(file->size());

    cout << "Starting calculations from n=" << n << endl;
    while (true) {
        auto now = high_resolution_clock::now();
        if (now >= next_display_time) {
            auto elapsed = duration_cast<milliseconds>(now - start);
            cout << "Time elapsed: " << elapsed.count() << " millisecond(s). ";
            cout << "Last calculated n=" << n - 1 << endl;
            next_display_time += display_interval;
        }
        if (now - start > time_limit)
            break;

        calculator.calculate_nim(Caterpillar(n, x_class));
        n++;
    }

    cout << "End of calculations. Last calculated n=" << n - 1 << endl;

    std::cout << std::endl;
}

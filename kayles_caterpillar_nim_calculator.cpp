#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

typedef unsigned int uint;
#define tx vector<bool>


class Caterpillar {
public:
    int n;
    tx x;

    Caterpillar(int n, tx &x)
    : n(n), x(max(0, n - 2)) {
        int j = min(n - 2, (int) x.size());
        for (int i=0; i < j; i++)
            this->x[i] = x[i];
    }

    Caterpillar(int n, uint x)
    : n(n), x(max(0, n - 2)) {
        int j = min(this->x.size(), sizeof(x) * 8);
        for (int i=0; i < j; i++)
            this->x[i] = x & (1 << i);
    }

    uint get_x_class() {
        uint x_class = 0;
        for (size_t i=0; i<this->x.size(); i++)
            x_class += (this->x[i]) ? (1 << i) : 0;
        return x_class;
    }

    static Caterpillar open_right(int n, tx &x) {
        // recebe um Caterpillar sem vértice normal "0" à direita
        // retorna ele com vértice 0 à direita
        // exemplo: 0(010101011101 -> 0(010101011100)0
        if (n == 1) return Caterpillar(1, 0);
        if (x.back()) {
            tx x1(x.begin(), x.end());
            x1.back() = false;
            return Caterpillar(n + 1, x1);
        }
        tx x1(x.begin(), x.end() - 1);
        return Caterpillar(n, x1);
    }

    static Caterpillar open_left(int n, tx &x) {
        // recebe um Caterpillar sem vértice normal "0" à esquerda
        // retorna ele com vértice 0 à esquerda
        // exemplo: 010101011101)0 -> 0(10101011101)0
        if (n == 1) return Caterpillar(1, 0);
        if (x.front()) {
            tx x1(x.begin(), x.end());
            x1.front() = false;
            return Caterpillar(n + 1, x1);
        }
        tx x1(x.begin() + 1, x.end());
        return Caterpillar(n, x1);
    }

    void print() {
        cout << 0;
        for (bool xi : this->x)
            cout << (xi ? 1 : 0);
        cout << 0;
    }
};

class CaterpillarNimFile {
    string filename;
    fstream file;
    uint x_class;
    size_t offset;
public:
    CaterpillarNimFile(string filename, uint x_class) {
        this->filename = filename;
        this->x_class = x_class;
        this->offset = (x_class == 0) ? 0 : 3 + log2(x_class);

        this->file.open(filename, ios::in | ios::out | ios::binary | ios::app);

        if (!this->file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            return;
        }
    }

    ~CaterpillarNimFile() {
        this->file.close();
    }
    
    size_t size() {
        this->file.seekg(0, ios::end);
        return file.tellg() / sizeof(uint);
    }

    size_t get_pos(uint n) {
        if (n < offset) return 0;
        return n - offset;
    }

    int get_n(size_t pos) {
        return pos + offset;
    }

    uint read(size_t pos) {
        this->file.seekg(pos * sizeof(uint));
        uint nim;
        this->file.read((char*)&nim, sizeof(uint));
        return nim;
    }

    uint nim(uint n) {
        return read(get_pos(n));
    }

    uint is_calculated(uint n) {
        return get_pos(n) < this->size();
    }

    void write(uint nim) {
        this->file.seekp(0, ios::end);
        this->file.write((char*)&nim, sizeof(uint));
    }

    void write(uint n, uint nim) {
        size_t pos = get_pos(n);
        this->file.seekp(pos * sizeof(uint));
        this->file.write((char*)&nim, sizeof(uint));
    }
};

class CaterpillarNimFileIterator {
    CaterpillarNimFile *file;
    uint position;
public:
    CaterpillarNimFileIterator(CaterpillarNimFile *file) {
        this->file = file;
        this->position = 0;
    }

    void reset() {
        this->position = 0;
    }

    uint next() {
        return this->file->read(this->position++);
    }

    bool has_next() {
        return this->position < this->file->size();
    }
};


class CaterpillarNimCalculator {
    vector<CaterpillarNimFile*> files;
    string file_prefix;
public:
    CaterpillarNimCalculator(string file_prefix) {
        this->file_prefix = file_prefix;

        CaterpillarNimFile *file0 = new CaterpillarNimFile(file_prefix + "0", 0);
        file0->write(0, 0);
        file0->write(1, 1);
        files.push_back(file0);
    }

    ~CaterpillarNimCalculator() {
        for (auto file : files)
            delete file;
    }

    CaterpillarNimFile* get_file(uint x) {
        for (int i=files.size(); i <= (int) x; i++)
            files.push_back(new CaterpillarNimFile(
                file_prefix + (to_string(i)),
                i
            ));
        
        return files[x];
    }

    uint calculate_play_nim(Caterpillar &c, int i, bool p) {
        /*
        c: Caterpillar jogado
        i: vértice jogado
        p: jogada no vértice caminho / no vértice solto
            (se a jogada não for possível, será jogado no caminho)
        */
        
        if (c.n == 1 || c.n == 2)
            return calculate_nim(Caterpillar(0, 0));

        if (i == 0) {
            tx x1(c.x.begin() + 1, c.x.end());
            Caterpillar c1 = Caterpillar::open_left(c.n - 2, x1);
            uint nim = calculate_nim(c1);
            if (c.x.front())
                nim ^= calculate_nim(Caterpillar(1, 0));
            return nim;
        }

        if (i == c.n - 1) {
            tx x1(c.x.begin(), c.x.end() - 1);
            Caterpillar c1 = Caterpillar::open_right(c.n - 2, x1);
            uint nim = calculate_nim(c1);
            if (c.x.back())
                nim ^= calculate_nim(Caterpillar(1, 0));
            return nim;
        }

        if (i == 1) {
            if (p || !c.x.front()) {
                tx x1(c.x.begin() + 2, c.x.end());
                Caterpillar c1 = Caterpillar::open_left(c.n - 3, x1);
                uint nim = calculate_nim(c1);
                if (c.x[1])
                    nim ^= calculate_nim(Caterpillar(1, 0));
                return nim;
            }

            tx x1(c.x.begin() + 1, c.x.end());
            Caterpillar c1 = Caterpillar::open_left(c.n - 2, x1);
            return calculate_nim(c1) ^ calculate_nim(Caterpillar(1, 0));
        }

        if (i == c.n - 2) {
            if (p || !c.x.back()) {
                tx x1(c.x.begin(), c.x.end() - 2);
                Caterpillar c1 = Caterpillar::open_right(c.n - 3, x1);
                uint nim = calculate_nim(c1);
                if (*(c.x.rbegin() + 1))
                    nim ^= calculate_nim(Caterpillar(1, 0));
                return nim;
            }

            tx x1(c.x.begin(), c.x.end() - 1);
            Caterpillar c1 = Caterpillar::open_right(c.n - 2, x1);
            return calculate_nim(c1) ^ calculate_nim(Caterpillar(1, 0));
        }

        i--;  // ajusta i para usar como índice de x
        if (p || !c.x[i]) {
            tx x1(c.x.begin(), c.x.begin() + (i - 1));
            Caterpillar c1 = Caterpillar::open_right(i, x1);
            tx x2(c.x.begin() + (i + 2), c.x.end());
            Caterpillar c2 = Caterpillar::open_left(c.n - (i + 3), x2);

            uint nim = calculate_nim(c1) ^ calculate_nim(c2);
            if (c.x[i + 1])
                nim ^= calculate_nim(Caterpillar(1, 0));
            if (c.x[i - 1])
                nim ^= calculate_nim(Caterpillar(1, 0));

            return nim;
        } else {
            tx x1(c.x.begin(), c.x.begin() + i);
            Caterpillar c1 = Caterpillar::open_right(i + 1, x1);
            tx x2(c.x.begin() + (i + 1), c.x.end());
            Caterpillar c2 = Caterpillar::open_left(c.n - (i + 2), x2);
            return calculate_nim(c1) ^ calculate_nim(c2);
        }
    }

    uint calculate_nim(Caterpillar c) {
        CaterpillarNimFile *x_file = get_file(c.get_x_class());
        if (x_file->is_calculated(c.n))
            return x_file->nim(c.n);

        if (c.n > x_file->get_n(0))
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
        
        x_file->write(nim);
        return nim;
    }
};


void run_tests(CaterpillarNimCalculator &calculator) {
    map<pair<int, tx>, pair<int, tx>> open_right_tests = {
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
        if (c.n == expected.first && c.x == expected.second) {
            cout << "Sucess";
        } else {
            cout << "Failed. Expected: ";
            expected_c.print();
            cout << " Found: ";
            c.print();
        }
        cout << endl;
    }

    map<pair<int, tx>, pair<int, tx>> open_left_tests = {
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
        if (c.n == expected.first && c.x == expected.second) {
            cout << "Sucess";
        } else {
            cout << "Failed. Expected: ";
            expected_c.print();
            cout << " Found: ";
            c.print();
        }
        cout << endl;
    }

    map<int, uint> nim_tests = {
        {3, 1}, {4, 3}, {5, 0}, {6, 0}, {7, 1}
    };
    cout << "NIM TESTS" << endl;
    for (auto test : nim_tests) {
        cout << "Calculating for n=" << test.first << ") - ";
        uint nim = calculator.calculate_nim(Caterpillar(test.first, 1));
        if (nim == test.second) {
            cout << "Success";
        } else {
            cout << "Failed. Expected: " << test.second << ", Found: " << nim;
        }
        cout << endl;
    }
}

void calculate_by_time(
    CaterpillarNimCalculator &calculator,
    uint x_class,
    const chrono::milliseconds &time_limit,
    const chrono::milliseconds &display_interval = minutes(1)
) {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    auto next_display_time = start + display_interval;

    CaterpillarNimFile *file = calculator.get_file(x_class);

    int n = file->get_n(file->size());
    cout << "Starting calculations from n=" << n << endl;

    while (true) {
        auto now = high_resolution_clock::now();
        if (now - start > time_limit)
            break;

        calculator.calculate_nim(Caterpillar(n, x_class));
        n++;

        if (now >= next_display_time) {
            auto elapsed = duration_cast<milliseconds>(now - start);
            cout << "Time elapsed: " << elapsed.count() << " millisecond(s). ";
            cout << "Last calculated n=" << n - 1 << endl;
            next_display_time += display_interval;
        }
    }

    cout << "End of calculations. Last calculated n=" << n - 1 << endl;

    std::cout << std::endl;
}

int main() {
    CaterpillarNimCalculator calculator("caterpillar_nim_");

    // run_tests(calculator);
    calculate_by_time(calculator, 1, seconds(15), seconds(1));

    return 0;
}

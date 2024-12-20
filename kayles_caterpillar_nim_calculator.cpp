#include <iostream>
#include <fstream>
#include <boost/dynamic_bitset.hpp>
#include <filesystem>
#include <chrono>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <math.h>

using namespace std;
using namespace std::chrono;

typedef unsigned int uint;
#define DEFAULT_CACHE_SIZE (1 << 20)
#define DEFAULT_MAX_OPEN_FILE 10


class Caterpillar {
public:
    int n;
    boost::dynamic_bitset<> x;
    uint x_class;

    Caterpillar(int n, uint x)
    : n(n), x(max(0, n - 2)) {
        int j = min(this->x.size(), sizeof(x) * 8);
        for (int i = 0; i < j; i++)
            this->x[i] = x & (1 << i);

        this->x_class = x;
    }

    Caterpillar(int n, const boost::dynamic_bitset<> &x)
    : n(n), x(x) {
        x_class = 0;
        for (size_t i = 0; i < x.size(); i++)
            x_class += (x[i]) ? (1 << i) : 0;
    }

    uint get_x_class() const {
        return x_class;
    }

    static Caterpillar open_right(int n, const boost::dynamic_bitset<> &x) {
        if (n == 1) return Caterpillar(1, 0);
        boost::dynamic_bitset<> x1 = x;
        if (x.test(n - 2)) {
            x1.reset(n - 2);
            return Caterpillar(n + 1, x1);
        }
        return Caterpillar(n, x1 >> 1);
    }

    static Caterpillar open_left(int n, const boost::dynamic_bitset<> &x) {
        if (n == 1) return Caterpillar(1, 0);
        boost::dynamic_bitset<> x1 = x;
        if (x.test(0)) {
            x1.reset(0);
            return Caterpillar(n + 1, x1);
        }
        return Caterpillar(n, x1 << 1);
    }

    void print() const {
        std::cout << 0 << "(";
        for (size_t i = 0; i < x.size(); ++i)
            std::cout << x.test(i);
        std::cout << ")" << 0;
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
    CaterpillarNimFile(
        string filename,
        uint x_class,
        size_t cache_size = DEFAULT_CACHE_SIZE,
        bool start_open = true
    ) {
        this->filename = filename;
        this->x_class = x_class;
        this->offset = (x_class == 0) ? 0 : 3 + log2(x_class);
        this->cache_size = cache_size;
        this->cache = vector<uint>(cache_size);

        filesystem::create_directories(filesystem::path(filename).parent_path());
        open();
        file.seekg(0, ios::end);
        file_size = file.tellg();

        int cache_loaded = 0;
        for (int i=0; i<cache_size && i<size(); i++, cache_loaded++)
            cache[i] = read(i);
        cout << "Cache loaded: " << cache_loaded << " of " << cache_size << endl;
        
        if (!start_open)
            close();
    }

    ~CaterpillarNimFile() {
        if (is_open())
            close();
    }

    void open() {
        cout << "Opening file: " << filename << endl;

        file.open(filename, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            cout << "Failed. Trying to create file: " << filename << endl;
            file.clear();
            file.open(filename, ios::out | ios::binary);
            file.close();
            cout << "Reopening file: " << filename << endl;
            file.open(filename, ios::in | ios::out | ios::binary);
        }
        if (!is_open()) {
            cerr << "Error opening file: " << filename << endl;
            exit(EXIT_FAILURE);
        }
    }

    bool is_open() {
        return file.is_open();
    }

    void close() {
        if (!is_open()) {
            cout << "File already closed: " << filename << endl;
            return;
        }
        cout << "Closing file: " << filename << endl;
        file.close();
    }
    
    size_t size() {
        return file_size / sizeof(uint);
    }

    size_t get_pos(uint n) {
        if (n < offset)
            return 0;
        return n - offset;
    }

    int get_n(size_t pos) {
        return pos + offset;
    }

    uint read(size_t pos) {
        if (!this->is_open()) {
            cerr << "File not open" << endl;
            exit(EXIT_FAILURE);
        }

        uint nim;
        file.seekg(pos * sizeof(uint));
        file.read((char*)&nim, sizeof(uint));
        return nim;
    }

    void write(size_t pos, uint nim) {
        if (!this->is_open()) {
            cerr << "File not open" << endl;
            exit(EXIT_FAILURE);
        }


        if (pos >= this->size()) {
            pos = this->size();
            file_size += sizeof(uint);
        }

        if (pos < this->cache_size)
            this->cache[pos] = nim;

        this->file.seekp(pos * sizeof(uint));
        this->file.write((char*)&nim, sizeof(uint));
    }

    void write_n(uint n, uint nim) {
        write(get_pos(n), nim);
    }

    uint read_cached(size_t pos) {
        if (pos < this->cache_size)
            return this->cache[pos];

        return read(pos);
    }

    uint read_n(uint n) {
        return read_cached(get_pos(n));
    }

    uint is_calculated(uint n) {
        return get_pos(n) < this->size();
    }

    bool is_cached(uint n) {
        return get_pos(n) < this->cache_size;
    }

    void write(uint nim) {
        this->write(this->size(), nim);
    }
};

class CaterpillarNimFileManager {
    queue<uint> open_files_queue;
    set<uint> open_files;
    vector<CaterpillarNimFile*> files;
    string file_prefix;
    int max_open_file;
    size_t default_cache_size;
public:
    CaterpillarNimFileManager(
        string file_prefix,
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    ) {
        this->file_prefix = file_prefix;
        this->max_open_file = max_open_file;
        this->default_cache_size = default_cache_size;
    }

    ~CaterpillarNimFileManager() {
        for (auto file : files)
            delete file;
    }

    void open_file(uint x) {
        if (open_files.find(x) == open_files.end()) {
            files[x]->open();
            open_files_queue.emplace(x);
            open_files.insert(x);

            if (open_files_queue.size() > max_open_file) {
                uint x_to_close = open_files_queue.front();
                open_files_queue.pop();
                files[x_to_close]->close();
                open_files.erase(x_to_close);
            }
        }
    }

    CaterpillarNimFile* get_file(uint x) {
        for (uint i=files.size(); i <= x; i++)
            files.push_back(new CaterpillarNimFile(
                file_prefix + (to_string(i)),
                i,
                default_cache_size,
                false
            ));

        return files[x];
    }

    void close() {
        while (!open_files.empty()) {
            uint x = open_files_queue.front();
            open_files_queue.pop();
            files[x]->close();
        }

        open_files.clear();
    }
};

class CaterpillarNimCalculator {
public:
    CaterpillarNimFileManager* file_manager;
    CaterpillarNimCalculator(
        string file_prefix, 
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    ) {
        this->file_manager = new CaterpillarNimFileManager(file_prefix, max_open_file, default_cache_size);

        CaterpillarNimFile *file0 = file_manager->get_file(0);
        file_manager->open_file(0);
        file0->write(0, 0);
        file0->write(1, 1);
    }

    ~CaterpillarNimCalculator() {
        delete file_manager;
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
            boost::dynamic_bitset<> x1(c.x.size() - 1);
            for (size_t j = 1; j < c.x.size(); ++j)
                x1[j - 1] = c.x[j];
            Caterpillar c1 = Caterpillar::open_left(c.n - 2, x1);
            uint nim = calculate_nim(c1);
            if (c.x.test(0))
                nim ^= calculate_nim(Caterpillar(1, 0));
            return nim;
        }

        if (i == c.n - 1) {
            boost::dynamic_bitset<> x1(c.x.size() - 1);
            for (size_t j = 0; j < c.x.size() - 1; ++j)
                x1[j] = c.x[j];
            Caterpillar c1 = Caterpillar::open_right(c.n - 2, x1);
            uint nim = calculate_nim(c1);
            if (c.x.test(c.x.size() - 1))
                nim ^= calculate_nim(Caterpillar(1, 0));
            return nim;
        }

        if (i == 1) {
            if (p || !c.x.test(0)) {
                boost::dynamic_bitset<> x1(c.x.size() - 2);
                for (size_t j = 2; j < c.x.size(); ++j)
                    x1[j - 2] = c.x[j];
                Caterpillar c1 = Caterpillar::open_left(c.n - 3, x1);
                uint nim = calculate_nim(c1);
                if (c.x[1])
                    nim ^= calculate_nim(Caterpillar(1, 0));
                return nim;
            }

            boost::dynamic_bitset<> x1(c.x.size() - 1);
            for (size_t j = 1; j < c.x.size(); ++j)
                x1[j - 1] = c.x[j];
            Caterpillar c1 = Caterpillar::open_left(c.n - 2, x1);
            return calculate_nim(c1) ^ calculate_nim(Caterpillar(1, 0));
        }

        if (i == c.n - 2) {
            if (p || !c.x.test(c.x.size() - 1)) {
                boost::dynamic_bitset<> x1(c.x.size() - 2);
                for (size_t j = 0; j < c.x.size() - 2; ++j)
                    x1[j] = c.x[j];
                Caterpillar c1 = Caterpillar::open_right(c.n - 3, x1);
                uint nim = calculate_nim(c1);
                if (c.x[c.x.size() - 2])
                    nim ^= calculate_nim(Caterpillar(1, 0));
                return nim;
            }

            boost::dynamic_bitset<> x1(c.x.size() - 1);
            for (size_t j = 0; j < c.x.size() - 1; ++j)
                x1[j] = c.x[j];
            Caterpillar c1 = Caterpillar::open_right(c.n - 2, x1);
            return calculate_nim(c1) ^ calculate_nim(Caterpillar(1, 0));
        }

        i--;  // ajusta i para usar como índice de x
        if (p || !c.x[i]) {
            boost::dynamic_bitset<> x1(i);
            for (int j = 0; j < i; ++j)
                x1[j] = c.x[j];
            Caterpillar c1 = Caterpillar::open_right(i, x1);

            boost::dynamic_bitset<> x2(c.x.size() - (i + 2));
            for (size_t j = i + 2; j < c.x.size(); ++j)
                x2[j - (i + 2)] = c.x[j];
            Caterpillar c2 = Caterpillar::open_left(c.n - (i + 3), x2);

            uint nim = calculate_nim(c1) ^ calculate_nim(c2);
            if (c.x[i + 1])
                nim ^= calculate_nim(Caterpillar(1, 0));
            if (c.x[i - 1])
                nim ^= calculate_nim(Caterpillar(1, 0));

            return nim;
        } else {
            boost::dynamic_bitset<> x1(i + 1);
            for (int j = 0; j <= i; ++j)
                x1[j] = c.x[j];
            Caterpillar c1 = Caterpillar::open_right(i + 1, x1);

            boost::dynamic_bitset<> x2(c.x.size() - (i + 1));
            for (size_t j = i + 1; j < c.x.size(); ++j)
                x2[j - (i + 1)] = c.x[j];
            Caterpillar c2 = Caterpillar::open_left(c.n - (i + 2), x2);
            return calculate_nim(c1) ^ calculate_nim(c2);
        }
    }

    uint calculate_nim(const Caterpillar &c) {
        CaterpillarNimFile *file;
        file = file_manager->get_file(c.get_x_class());
        if (file->is_calculated(c.n)) {
            if (!file->is_cached(c.n))
                file_manager->open_file(c.get_x_class());
            return file->read_n(c.n);
        }

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

        file_manager->open_file(c.get_x_class());
        file->write(nim);
        return nim;
    }
};

void calculate_by_n(
    CaterpillarNimCalculator &calculator,
    uint x_class,
    uint n_limit,
    const milliseconds &display_interval = minutes(1)
) {
    auto start = high_resolution_clock::now();
    auto next_display_time = start + display_interval;

    CaterpillarNimFile *file = calculator.file_manager->get_file(x_class);

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

    CaterpillarNimFile *file = calculator.file_manager->get_file(x_class);

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

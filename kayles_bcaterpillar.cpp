#ifndef KAYLES_BCATERPILLAR_H
#define KAYLES_BCATERPILLAR_H

#include "kayles_caterpillar.cpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <queue>
#include <set>
#include <math.h>
#include <functional>

class BCaterpillar : public Caterpillar {
protected:
    unsigned int x_class;
public:
    BCaterpillar(int n, unsigned int x)
    : Caterpillar(n) {
        int j = std::min(n - 2, (int) sizeof(x) * 8);
        this->x_class = 0;
        for (int i = 0; i < j; i++) {
            this->x[i + 1] = x & (1 << i);
            this->x_class += (this->x[i + 1]) ? (1 << i) : 0;
        }
    }

    BCaterpillar(int n)
    : Caterpillar(n) {
        this->x_class = 0;
    }

    BCaterpillar(std::vector<int> x)
    : Caterpillar(x) {
        this->x_class = 0;
        for (int i=1; i<x.size() - 1; i++) {
            if (x[i]) {
                x[i] = 1;
                this->x_class += 1 << (i - 1);
            }
        }
    }

    virtual ~BCaterpillar() {}

    unsigned int get_x_class() const {
        return x_class;
    }
};

class BCaterpillarFactory : public AbstractCaterpillarFactory {
public:
    Caterpillar* create(int n) override {
        return new BCaterpillar(n);
    }

    Caterpillar* create(std::vector<int> x) override {
        return new BCaterpillar(x);
    }
};


#define DEFAULT_CACHE_SIZE (1 << 20)
#define DEFAULT_MAX_OPEN_FILE 10

class BCaterpillarNimFile {
    std::string filename;
    std::fstream file;
    size_t file_size;
    unsigned int x_class;
    size_t offset;
    size_t cache_size;
    std::vector<unsigned int> cache;
public:
    BCaterpillarNimFile(
        std::string filename,
        unsigned int x_class,
        size_t cache_size = DEFAULT_CACHE_SIZE,
        bool start_open = true
    ) {
        this->filename = filename;
        this->x_class = x_class;
        this->offset = (x_class == 0) ? 0 : 3 + log2(x_class);
        this->cache_size = cache_size;
        this->cache = std::vector<unsigned int>(cache_size);

        std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
        open();
        file.seekg(0, std::ios::end);
        file_size = file.tellg();

        int cache_loaded = 0;
        for (int i=0; i<cache_size && i<size(); i++, cache_loaded++)
            cache[i] = read(i);
        std::cout << "Cache loaded: " << cache_loaded << " of " << cache_size << "\n";
        
        if (!start_open)
            close();
    }

    ~BCaterpillarNimFile() {
        if (is_open())
            close();
    }

    void open() {
        std::cout << "Opening file: " << filename << "\n";

        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed. Trying to create file: " << filename << "\n";
            file.clear();
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            std::cout << "Reopening file: " << filename << "\n";
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        }
        if (!is_open()) {
            std::cerr << "Error opening file: " << filename << "\n";
            exit(EXIT_FAILURE);
        }
    }

    bool is_open() {
        return file.is_open();
    }

    void close() {
        if (!is_open()) {
            std::cout << "File already closed: " << filename << "\n";
            return;
        }
        std::cout << "Closing file: " << filename << "\n";
        file.close();
    }
    
    size_t size() {
        return file_size / sizeof(unsigned int);
    }

    size_t get_pos(unsigned int n) {
        if (n < offset)
            return 0;
        return n - offset;
    }

    int get_n(size_t pos) {
        return pos + offset;
    }

    unsigned int read(size_t pos) {
        if (!this->is_open()) {
            std::cerr << "File not open" << "\n";
            exit(EXIT_FAILURE);
        }

        unsigned int nim;
        file.seekg(pos * sizeof(unsigned int));
        file.read((char*)&nim, sizeof(unsigned int));
        return nim;
    }

    void write(size_t pos, unsigned int nim) {
        if (!this->is_open()) {
            std::cerr << "File not open" << "\n";
            exit(EXIT_FAILURE);
        }

        if (pos >= this->size()) {
            pos = this->size();
            file_size += sizeof(unsigned int);
        }

        if (pos < this->cache_size)
            this->cache[pos] = nim;

        this->file.seekp(pos * sizeof(unsigned int));
        this->file.write((char*)&nim, sizeof(unsigned int));
    }

    void write_n(unsigned int n, unsigned int nim) {
        write(get_pos(n), nim);
    }

    unsigned int read_cached(size_t pos) {
        if (pos < this->cache_size)
            return this->cache[pos];

        return read(pos);
    }

    unsigned int read_n(unsigned int n) {
        return read_cached(get_pos(n));
    }

    unsigned int is_calculated(unsigned int n) {
        return get_pos(n) < this->size();
    }

    bool is_cached(unsigned int n) {
        return get_pos(n) < this->cache_size;
    }

    void write(unsigned int nim) {
        this->write(this->size(), nim);
    }
};

class BCaterpillarNimFileManager {
    std::queue<unsigned int> open_files_queue;
    std::set<unsigned int> open_files;
    std::vector<BCaterpillarNimFile*> files;
    std::string file_prefix;
    int max_open_file;
    size_t default_cache_size;
public:
    BCaterpillarNimFileManager(
        std::string file_prefix,
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    ) {
        this->file_prefix = file_prefix;
        this->max_open_file = max_open_file;
        this->default_cache_size = default_cache_size;
    }

    ~BCaterpillarNimFileManager() {
        for (auto file : files)
            delete file;
    }

    void open_file(unsigned int x) {
        if (open_files.find(x) == open_files.end()) {
            files[x]->open();
            open_files_queue.emplace(x);
            open_files.insert(x);

            if (open_files_queue.size() > max_open_file) {
                unsigned int x_to_close = open_files_queue.front();
                open_files_queue.pop();
                files[x_to_close]->close();
                open_files.erase(x_to_close);
            }
        }
    }

    BCaterpillarNimFile* get_file(unsigned int x) {
        for (unsigned int i=files.size(); i <= x; i++)
            files.push_back(new BCaterpillarNimFile(
                file_prefix + (std::to_string(i)),
                i,
                default_cache_size,
                false
            ));

        return files[x];
    }

    void close() {
        while (!open_files.empty()) {
            unsigned int x = open_files_queue.front();
            open_files_queue.pop();
            files[x]->close();
        }

        open_files.clear();
    }
};

class BCaterpillarNimCalculator : public CaterpillarNimCalculator {
public:
    BCaterpillarNimFileManager* file_manager;
    BCaterpillarNimCalculator(
        std::string file_prefix, 
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    ) : CaterpillarNimCalculator(new BCaterpillarFactory())
    {
        this->file_manager = new BCaterpillarNimFileManager(file_prefix, max_open_file, default_cache_size);

        BCaterpillarNimFile *file0 = file_manager->get_file(0);
        file_manager->open_file(0);
        file0->write(0, 0);
        file0->write(1, 1);
    }

    ~BCaterpillarNimCalculator() {
        delete file_manager;
    }

    unsigned int calculate_nim(const Caterpillar *c) override {
        const BCaterpillar *bc = dynamic_cast<const BCaterpillar*>(c);
        if (bc == nullptr)
            return CaterpillarNimCalculator::calculate_nim(c);
        
        BCaterpillarNimFile *file;
        file = file_manager->get_file(bc->get_x_class());
        if (file->is_calculated(bc->size())) {
            if (!file->is_cached(bc->size()))
                file_manager->open_file(bc->get_x_class());
            return file->read_n(bc->size());
        }

        if (bc->size() > file->get_n(file->size())) {
            Caterpillar *c1 = new BCaterpillar(bc->size() - 1, bc->get_x_class());
            calculate_nim(c1);
            delete c1;
        }
        //  certifica-se que o BCaterpillar anterior já foi calculado
        //  para manter a sequencialidade do arquivo

        unsigned int nim = CaterpillarNimCalculator::calculate_nim(c);

        file_manager->open_file(bc->get_x_class());
        file->write(nim);
        return nim;
    }
};

void BCaterpillar_calculate(
    BCaterpillarNimCalculator* calculator,
    unsigned int x_class,
    const std::function<bool(int, std::chrono::milliseconds)> &stop_condition,
    const std::chrono::milliseconds &display_interval = std::chrono::minutes(1)
) {
    auto start = std::chrono::high_resolution_clock::now();
    auto next_display_time = start + display_interval;

    BCaterpillarNimFile *file = calculator->file_manager->get_file(x_class);

    int n = file->get_n(file->size());
    
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

    std::cout << "Starting calculations from n=" << n << "\n";
    while (!stop_condition(n, elapsed)) {
        if (now >= next_display_time) {
            std::cout << "Time elapsed: " << elapsed.count() << " millisecond(s). ";
            std::cout << "Last calculated n=" << n - 1 << "\n";
            next_display_time += display_interval;
        }

        Caterpillar *c = new BCaterpillar(n, x_class);
        calculator->calculate_nim(c);
        delete c;
        n++;

        now = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
    }

    std::cout << "End of calculations. Last calculated n=" << n - 1 << "\n";
}

void BCaterpillar_calculate_by_n(
    BCaterpillarNimCalculator *calculator,
    unsigned int x_class,
    unsigned int n_limit,
    const std::chrono::milliseconds &display_interval = std::chrono::minutes(1)
) {
    BCaterpillar_calculate(
        calculator,
        x_class,
        [n_limit](int n, std::chrono::milliseconds elapsed) {
            return n >= n_limit;
        },
        display_interval
    );
}

void BCaterpillar_calculate_by_time(
    BCaterpillarNimCalculator *calculator,
    unsigned int x_class,
    const std::chrono::milliseconds &time_limit,
    const std::chrono::milliseconds &display_interval = std::chrono::minutes(1)
) {
    BCaterpillar_calculate(
        calculator,
        x_class,
        [time_limit](int n, std::chrono::milliseconds elapsed) {
            return elapsed >= time_limit;
        },
        display_interval
    );
}

#endif // KAYLES_BCATERPILLAR_H

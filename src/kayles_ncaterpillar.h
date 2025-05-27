#ifndef KAYLES_NCATERPILLAR_H
#define KAYLES_NCATERPILLAR_H

#include "kayles_caterpillar.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <chrono>
#include <functional>
#include <memory>

template<unsigned int N_REDUCED>
class NCaterpillar : public Caterpillar {
protected:
    const unsigned int x_class;
public:
    NCaterpillar(unsigned int n)
        : Caterpillar(std::vector<unsigned int>(n, 0)), x_class(0) {}

    NCaterpillar(unsigned int n, unsigned int x)
        : Caterpillar([n, x] {
            std::vector<unsigned int> v(n, 0);
            unsigned int tmp = x;
            int i = 1;
            while (tmp > 0 && i < (int)v.size()) {
                v[i++] = tmp % N_REDUCED;
                tmp /= N_REDUCED;
            }
            return v;
        }()),
        x_class(x) {}

    NCaterpillar(std::vector<unsigned int> x)
        : Caterpillar([&x] {
            int lz = 0;
            int tz = 0;
            for (int i=0; i<x.size(); i++) {
                if (x[i]) break;
                lz++;
            }
            for (int i = x.size() - 1; i>=0; i--) {
                if (x[i]) break;
                tz++;
            }
            std::vector<unsigned int> v = (lz >= tz) ? (std::vector<unsigned int>(x.rbegin(), x.rend())) : x;

            for (int i = 1; i < (int)v.size() - 1; i++)
                v[i] = std::min(v[i], N_REDUCED - 1);
            if (!v.empty() && v.front() > 0) {
                v.front()--;
                v.insert(v.begin(), 0);
            }
            if (!v.empty() && v.back() > 0) {
                v.back()--;
                v.push_back(0);
            }
            return v;
        }()),
        x_class([&x] {
            unsigned int xc = 0;
            for (int i = 1; i < (int)x.size() - 1; i++)
                xc += std::min(x[i], N_REDUCED) * std::pow(N_REDUCED, i - 1);
            return xc;
        }()) {}

    NCaterpillar(const Caterpillar& c) : NCaterpillar(c.get_x()) {}
    
    unsigned int get_x_class() const { return x_class; }
};

template<unsigned int N_REDUCED>
class NCaterpillarFactory : public CaterpillarFactory {
    std::unique_ptr<Caterpillar> create(unsigned int n) override {
        return std::make_unique<NCaterpillar<N_REDUCED>>(n);
    }

    std::unique_ptr<Caterpillar> create(std::vector<unsigned int> x) override {
        return std::make_unique<NCaterpillar<N_REDUCED>>(x);
    }
};

#define NIM_FILE_EXTENSION_NAME ".catnim"
#define DEFAULT_CACHE_SIZE (1 << 20)
#define DEFAULT_MAX_OPEN_FILE 10

template<unsigned int N_REDUCED>
class NCaterpillarNimFile {
    const size_t HEADER_POS_N_REDUCED  = 0;
    const size_t HEADER_SIZE_N_REDUCED = sizeof(unsigned int);
    const size_t HEADER_POS_X_CLASS    = HEADER_POS_N_REDUCED + HEADER_SIZE_N_REDUCED;
    const size_t HEADER_SIZE_X_CLASS   = sizeof(unsigned int);
    const size_t HEADER_POS_N0         = HEADER_POS_X_CLASS + HEADER_SIZE_X_CLASS;
    const size_t HEADER_SIZE_N0        = sizeof(unsigned int);
    const size_t HEADER_SIZE           = HEADER_SIZE_N_REDUCED + HEADER_SIZE_X_CLASS + HEADER_SIZE_N0;

    const size_t NIM_SIZE              = sizeof(unsigned int);
    
    std::string filename;
    std::fstream file;
    size_t current_file_size;  // for checks while file is closed
    unsigned int x_class;
    size_t n0;
    size_t cache_size;
    std::vector<unsigned int> cache;

    unsigned int get_n0(unsigned int x_class) {
        int i = 0;
        for (; x_class > 0; i++)
            x_class /= N_REDUCED;

        return 2 + i;
    }

    void write_headers() {
        file.seekp(HEADER_POS_N_REDUCED);
        unsigned int n_reduced = N_REDUCED;
        file.write((char*)&n_reduced, HEADER_SIZE_N_REDUCED);

        file.seekp(HEADER_POS_X_CLASS);
        file.write((char*)&x_class, HEADER_SIZE_X_CLASS);

        file.seekp(HEADER_POS_N0);
        file.write((char*)&n0, HEADER_SIZE_N0);
    }

    void load_current_file_size() {
        file.seekg(0, std::ios::end);
        this->current_file_size = file.tellg();
    }

    size_t load_cache() {
        size_t cache_loaded = 0;
        for (int i=0; i<cache_size && i<size(); i++, cache_loaded++)
            cache[i] = read(i);

        return cache_loaded;
    }

    size_t seek_pos(size_t pos) {
        return HEADER_SIZE + pos * NIM_SIZE;
    }
public:
    NCaterpillarNimFile(
        std::string filename,
        unsigned int x_class,
        size_t cache_size = DEFAULT_CACHE_SIZE
    ) {
        this->filename = filename;
        this->x_class = x_class;
        this->n0 = get_n0(x_class);
        this->cache_size = cache_size;
        this->cache = std::vector<unsigned int>(cache_size);

        if (!open()) {
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            if (!open()) {
                std::cerr << "Could not open the file\n";
                exit(EXIT_FAILURE);
            };
        }

        write_headers();
        load_current_file_size();
        load_cache();
        close();
    }

    ~NCaterpillarNimFile() {
        close();
    }

    unsigned int get_x_class() const { return x_class; }
    
    size_t get_n0() const {
        return n0;
    }

    bool is_open() {
        return file.is_open();
    }

    bool open() {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        return is_open();
    }
    
    void close() {
        if (!is_open())
            return;

        file.close();
    }
    
    size_t size() {
        return (current_file_size - HEADER_SIZE) / NIM_SIZE;
    }
    
    size_t hash(unsigned int n) {
        if (n < n0) return 0;
        return n - n0;
    }
    
    unsigned int ihash(size_t index) {
        return index + n0;
    }
    
    bool is_calculated(unsigned int n) {
        return n >= n0 && hash(n) < this->size();
    }
    
    bool is_cached(unsigned int n) {
        return hash(n) < this->cache_size;
    }
    
    unsigned int read(size_t pos) {
        if (!this->is_open()) {
            std::cerr << "File not open\n";
            exit(EXIT_FAILURE);
        }

        unsigned int nim;
        file.seekg(seek_pos(pos));
        file.read((char*)&nim, NIM_SIZE);
        return nim;
    }

    void write(size_t pos, unsigned int nim) {
        if (!this->is_open()) {
            std::cerr << "File not open\n";
            exit(EXIT_FAILURE);
        }

        if (pos >= this->size()) {
            pos = this->size();
            this->current_file_size += NIM_SIZE;
        }

        if (pos < this->cache_size)
            this->cache[pos] = nim;

        this->file.seekp(seek_pos(pos));
        this->file.write((char*)&nim, NIM_SIZE);
    }

    void write(unsigned int nim) {
        this->write(this->size(), nim);
    }
    
    unsigned int read_cached(size_t pos) {
        if (pos < this->cache_size)
            return this->cache[pos];

        return read(pos);
    }
    
    unsigned int read_n(unsigned int n) {
        return read_cached(hash(n));
    }
    
    void write_n(unsigned int n, unsigned int nim) {
        write(hash(n), nim);
    }
};

template<unsigned int N_REDUCED>
class NCaterpillarNimFileManager {
    std::queue<unsigned int> open_files_queue;
    std::set<unsigned int> open_files;
    std::map<unsigned int, std::unique_ptr<NCaterpillarNimFile<N_REDUCED>>> files;
    std::string file_prefix;
    int max_open_file;
    size_t default_cache_size;
public:
    NCaterpillarNimFileManager(
        std::string file_prefix,
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    ) : file_prefix(file_prefix), max_open_file(max_open_file), default_cache_size(default_cache_size) {}

    NCaterpillarNimFile<N_REDUCED>& get_file(unsigned int x) {
        if (files.find(x) == files.end()) {
            files.emplace(
                x,
                std::make_unique<NCaterpillarNimFile<N_REDUCED>>(
                    file_prefix + (std::to_string(x)) + NIM_FILE_EXTENSION_NAME,
                    x,
                    default_cache_size
                )
            );
        }

        return *files[x];
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
};

template<unsigned int N_REDUCED>
class NCaterpillarNimCalculator : public CaterpillarNimCalculator {
protected:
    std::unique_ptr<NCaterpillarNimFileManager<N_REDUCED>> file_manager;
public:
    NCaterpillarNimCalculator(
        std::string file_prefix, 
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    ) : CaterpillarNimCalculator(std::make_unique<NCaterpillarFactory<N_REDUCED>>()),
        file_manager(std::make_unique<NCaterpillarNimFileManager<N_REDUCED>>(file_prefix, max_open_file, default_cache_size))
    {}

    NCaterpillarNimFileManager<N_REDUCED>& get_file_manager() const {
        return *file_manager;
    }

    unsigned int calculate_nim(const Caterpillar& c) {
        const std::unique_ptr<NCaterpillar<N_REDUCED>> nc = std::make_unique<NCaterpillar<N_REDUCED>>(c);
        
        NCaterpillarNimFile<N_REDUCED>& file = file_manager->get_file(nc->get_x_class());

        if (nc->size() < file.get_n0())
            return CaterpillarNimCalculator::calculate_nim(c);

        if (file.is_calculated(nc->size())) {
            if (!file.is_cached(nc->size()))
                file_manager->open_file(nc->get_x_class());
            
            return file.read_n(nc->size());
        }

        if (nc->size() > file.ihash(file.size()))
            calculate_nim(NCaterpillar<N_REDUCED>(nc->size() - 1, nc->get_x_class()));

        unsigned int nim = CaterpillarNimCalculator::calculate_nim(c);
        file_manager->open_file(nc->get_x_class());
        file.write(nim);
        return nim;
    }

    void calculate_until(
        unsigned int x_class,
        const std::function<bool(int, std::chrono::milliseconds)> &stop_condition,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        std::ostream& os = std::cout
    ) {
        auto start = std::chrono::high_resolution_clock::now();
        auto next_display_time = start + display_interval;

        NCaterpillarNimFile<N_REDUCED>& file = file_manager->get_file(x_class);
        file_manager->open_file(x_class);

        unsigned int n = file.ihash(file.size());
        
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

        os << "Starting calculations from n=" << n << "\n";
        while (!stop_condition(n, elapsed)) {
            if (now >= next_display_time) {
                os << "Time elapsed: " << elapsed.count() << " millisecond(s). ";
                os << "Last calculated n=" << n - 1 << "\n";
                next_display_time += display_interval;
            }

            calculate_nim(NCaterpillar<N_REDUCED>(n, x_class));
            n++;

            now = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        }
        os << "End of calculations. Last calculated n=" << n - 1 << "\n";
    }

    void calculate_by_n(
        unsigned int x_class,
        unsigned int n_limit,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        std::ostream& os = std::cout
    ) {
        calculate_until(
            x_class,
            [n_limit](unsigned int n, std::chrono::milliseconds elapsed) {
                return n >= n_limit;
            },
            display_interval,
            os
        );
    }

    void calculate_by_time(
        unsigned int x_class,
        const std::chrono::milliseconds &time_limit,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        std::ostream& os = std::cout
    ) {
        calculate_until(
            x_class,
            [time_limit](unsigned int n, std::chrono::milliseconds elapsed) {
                return elapsed >= time_limit;
            },
            display_interval,
            os
        );
    }
};

#endif // KAYLES_NCATERPILLAR_H

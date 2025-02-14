#ifndef KAYLES_NCATERPILLAR_H
#define KAYLES_NCATERPILLAR_H

#include "kayles_caterpillar.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <queue>
#include <set>
#include <math.h>
#include <functional>

template<int N_REDUCED>
class NCaterpillar : public Caterpillar {
protected:
    unsigned int x_class;
public:
    NCaterpillar(int n, unsigned int x)
    : Caterpillar(n) {
        this->x_class = x;
        int i = 1;
        while (x > 0) {
            if (i < this->x.size()) {
                this->x[i++] = x % N_REDUCED;
            } else {
                this->x.push_back(x % N_REDUCED);
            }
            x /= N_REDUCED;
        }
        if (this->x.back() > 0)
            this->x.push_back(0);
    }
    NCaterpillar(int n)
    : Caterpillar(n) {
        this->x_class = 0;
    }
    NCaterpillar(std::vector<int> x)
    : Caterpillar(x) {
        this->x_class = 0;
        for (int i=1; i<(int)x.size() - 1; i++) {
            this->x[i] = std::min(this->x[i], N_REDUCED);
            this->x_class += this->x[i] * std::pow(N_REDUCED, i - 1);
        }
    }
    NCaterpillar(const Caterpillar *c)
    : Caterpillar(c) {
        this->x_class = 0;
        for (int i=1; i<(int)this->x.size() - 1; i++) {
            this->x[i] = std::min(this->x[i], N_REDUCED);
            this->x_class += this->x[i] * std::pow(N_REDUCED, i - 1);
        }
    }

    unsigned int get_x_class() const {
        return x_class;
    }
};

template<int N_REDUCED>
class NCaterpillarFactory : public CaterpillarFactory {
    Caterpillar* create(int n) override {
        return new NCaterpillar<N_REDUCED>(n);
    }
    Caterpillar* create(std::vector<int> x) override {
        return new NCaterpillar<N_REDUCED>(x);
    }
};

#define DEFAULT_CACHE_SIZE (1 << 20)
#define DEFAULT_MAX_OPEN_FILE 10

template<int N_REDUCED>
class NCaterpillarNimFile {
    std::string filename;
    std::fstream file;
    size_t file_size;
    unsigned int x_class;
    size_t offset;
    size_t cache_size;
    std::vector<unsigned int> cache;
    VerboseClass *verb;
    unsigned int get_setup_offset(unsigned int x_class) {
        if (x_class == 0) return 0;
        
        int i = 0;
        for (; x_class > 0; i++)
            x_class /= N_REDUCED;

        return 2 + i;
    }
public:
    NCaterpillarNimFile(
        std::string filename,
        unsigned int x_class,
        size_t cache_size = DEFAULT_CACHE_SIZE,
        bool start_open = true,
        bool verbose = false
    ) {
        this->filename = filename;
        this->x_class = x_class;
        
        this->offset = get_setup_offset(x_class);

        this->cache_size = cache_size;
        this->cache = std::vector<unsigned int>(cache_size);
        this->verb = new VerboseClass(verbose);

        std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
        open();
        file.seekg(0, std::ios::end);
        file_size = file.tellg();

        int cache_loaded = 0;
        for (int i=0; i<cache_size && i<size(); i++, cache_loaded++)
            cache[i] = read(i);

        std::stringstream ss;
        ss << "Cache loaded: " << cache_loaded << " of " << cache_size << "\n";
        verb->print(ss.str());

        if (!start_open)
            close();
    }
    ~NCaterpillarNimFile() {
        if (is_open())
            close();
        delete verb;
    }

    size_t get_offset() const {
        return offset;
    }

    void open() {
        std::stringstream ss;
        ss << "Opening file: " << filename << "\n";
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            ss << "Failed. Trying to create file: " << filename << "\n";
            file.clear();
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            ss << "Reopening file: " << filename << "\n";
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        }
        verb->print(ss.str());

        if (!is_open()) {
            std::cerr << "Error opening file: " << filename << "\n";
            exit(EXIT_FAILURE);
        }
    }
    bool is_open() {
        return file.is_open();
    }
    void close() {
        std::stringstream ss;
        if (!is_open()) {
            ss << "File already closed: " << filename << "\n";
            verb->print(ss.str());
            return;
        }
        ss << "Closing file: " << filename << "\n";
        verb->print(ss.str());
        file.close();
    }
    size_t size() {
        return file_size / sizeof(unsigned int);
    }

    size_t hash(int n) {
        if (n < offset) return 0;
        return n - offset;
    }
    int ihash(size_t index) {
        return index + offset;
    }

    unsigned int is_calculated(unsigned int n) {
        return hash(n) < this->size();
    }
    bool is_cached(unsigned int n) {
        return hash(n) < this->cache_size;
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
        unsigned int read_cached(size_t pos) {
        if (pos < this->cache_size)
            return this->cache[pos];

        return read(pos);
    }
    unsigned int read_n(unsigned int n) {
        return read_cached(hash(n));
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
        write(hash(n), nim);
    }
    void write(unsigned int nim) {
        this->write(this->size(), nim);
    }
};

template<int N_REDUCED>
class NCaterpillarNimFileManager {
    std::queue<unsigned int> open_files_queue;
    std::set<unsigned int> open_files;
    std::vector<NCaterpillarNimFile<N_REDUCED>*> files;
    std::string file_prefix;
    int max_open_file;
    size_t default_cache_size;
    bool file_verbose;
    VerboseClass* verb;
public:
    NCaterpillarNimFileManager(
        std::string file_prefix,
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE,
        bool verbose = false,
        bool file_verbose = false
    ) {
        this->file_prefix = file_prefix;
        this->max_open_file = max_open_file;
        this->default_cache_size = default_cache_size;
        this->verb = new VerboseClass(verbose);
        this->file_verbose = file_verbose;
    }
    ~NCaterpillarNimFileManager() {
        for (auto file : files)
        delete file;
        delete verb;
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
    NCaterpillarNimFile<N_REDUCED>* get_file(unsigned int x) {
        for (unsigned int i=files.size(); i <= x; i++)
            files.push_back(new NCaterpillarNimFile<N_REDUCED>(
                file_prefix + (std::to_string(i)),
                i,
                default_cache_size,
                false,
                file_verbose
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

template<int N_REDUCED>
class NCaterpillarNimCalculator : public CaterpillarNimCalculator {
protected:
    NCaterpillarNimFileManager<N_REDUCED>* file_manager;
public:
    NCaterpillarNimCalculator(
        std::string file_prefix, 
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    ) : CaterpillarNimCalculator(new NCaterpillarFactory<N_REDUCED>())
    {
        this->file_manager = new NCaterpillarNimFileManager<N_REDUCED>(file_prefix, max_open_file, default_cache_size);

        NCaterpillarNimFile<N_REDUCED> *file0 = file_manager->get_file(0);
        file_manager->open_file(0);
        file0->write(0, 0);
        file0->write(1, 1);
    }

    ~NCaterpillarNimCalculator() {
        delete file_manager;
    }

    NCaterpillarNimFileManager<N_REDUCED>* get_file_manager() const {
        return file_manager;
    }

    unsigned int calculate_nim(const Caterpillar *c, const VerboseClass &verb = VerboseClass(false)) {
        const NCaterpillar<N_REDUCED> *bc = new NCaterpillar<N_REDUCED>(c);
        
        NCaterpillarNimFile<N_REDUCED> *file;
        file = file_manager->get_file(bc->get_x_class());
        if (file->is_calculated(bc->size())) {
            if (!file->is_cached(bc->size()))
                file_manager->open_file(bc->get_x_class());
            return file->read_n(bc->size());
        }

        if (bc->size() > file->ihash(file->size())) {
            Caterpillar *c1 = new NCaterpillar<N_REDUCED>(bc->size() - 1, bc->get_x_class());
            calculate_nim(c1);
            delete c1;
        }
        //  certifica-se que o BCaterpillar anterior já foi calculado
        //  para manter a sequencialidade do arquivo

        unsigned int nim = CaterpillarNimCalculator::calculate_nim(c, verb);

        file_manager->open_file(bc->get_x_class());
        file->write(nim);
        return nim;
    }

    void calculate_until(
        unsigned int x_class,
        const std::function<bool(int, std::chrono::milliseconds)> &stop_condition,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        const VerboseClass &verb = VerboseClass(false)
    ) {
        auto start = std::chrono::high_resolution_clock::now();
        auto next_display_time = start + display_interval;

        NCaterpillarNimFileManager<N_REDUCED> *file_manager = get_file_manager();
        NCaterpillarNimFile<N_REDUCED> *file = file_manager->get_file(x_class);

        int n = file->ihash(file->size());
        
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

        std::stringstream ss;
        ss << "Starting calculations from n=" << n << "\n";

        while (!stop_condition(n, elapsed)) {
            if (now >= next_display_time) {
                ss << "Time elapsed: " << elapsed.count() << " millisecond(s). ";
                ss << "Last calculated n=" << n - 1 << "\n";
                next_display_time += display_interval;
            }
            verb.print(ss.str());
            ss.clear();

            Caterpillar *c = new NCaterpillar<N_REDUCED>(n, x_class);
            calculate_nim(c);
            delete c;
            n++;

            now = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        }

        ss << "End of calculations. Last calculated n=" << n - 1 << "\n";
        verb.print(ss.str());
    }

    void calculate_by_n(
        unsigned int x_class,
        unsigned int n_limit,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        const VerboseClass &verb = VerboseClass(false)
    ) {
        calculate_until(
            x_class,
            [n_limit](int n, std::chrono::milliseconds elapsed) {
                return n >= n_limit;
            },
            display_interval,
            verb
        );
    }

    void calculate_by_time(
        unsigned int x_class,
        const std::chrono::milliseconds &time_limit,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        const VerboseClass &verb = VerboseClass(false)
    ) {
    calculate_until(
            x_class,
            [time_limit](int n, std::chrono::milliseconds elapsed) {
                return elapsed >= time_limit;
            },
            display_interval,
            verb
        );
    }
};

#endif // KAYLES_NCATERPILLAR_H

#ifndef KAYLES_BCATERPILLAR_H
#define KAYLES_BCATERPILLAR_H

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

class BCaterpillar : public Caterpillar {
protected:
    unsigned int x_class;
public:
    BCaterpillar(int n, unsigned int x);
    BCaterpillar(int n);
    BCaterpillar(std::vector<int> x);
    BCaterpillar(const Caterpillar* c);
    virtual ~BCaterpillar();

    unsigned int get_x_class() const;
};

class BCaterpillarFactory : public AbstractCaterpillarFactory {
public:
    Caterpillar* create(int n) override;
    Caterpillar* create(std::vector<int> x) override;
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
    VerboseClass *verb;
public:
    BCaterpillarNimFile(
        std::string filename,
        unsigned int x_class,
        size_t cache_size = DEFAULT_CACHE_SIZE,
        bool start_open = true,
        bool verbose = false
    );
    ~BCaterpillarNimFile();

    size_t get_offset() const;

    void open();
    bool is_open();
    void close();
    size_t size();
    size_t get_pos(unsigned int n);
    int get_n(size_t pos);
    unsigned int read(size_t pos);
    void write(size_t pos, unsigned int nim);
    void write_n(unsigned int n, unsigned int nim);
    unsigned int read_cached(size_t pos);
    unsigned int read_n(unsigned int n);
    unsigned int is_calculated(unsigned int n);
    bool is_cached(unsigned int n);
    void write(unsigned int nim);
};

class BCaterpillarNimFileManager {
    std::queue<unsigned int> open_files_queue;
    std::set<unsigned int> open_files;
    std::vector<BCaterpillarNimFile*> files;
    std::string file_prefix;
    int max_open_file;
    size_t default_cache_size;
    bool file_verbose;
    VerboseClass* verb;
public:
    BCaterpillarNimFileManager(
        std::string file_prefix,
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE,
        bool verbose = false,
        bool file_verbose = false
    );
    ~BCaterpillarNimFileManager();

    void open_file(unsigned int x);
    BCaterpillarNimFile* get_file(unsigned int x);
    void close();
};

class BCaterpillarNimCalculator : public CaterpillarNimCalculator {
protected:
    BCaterpillarNimFileManager* file_manager;
public:
    BCaterpillarNimCalculator(
        std::string file_prefix, 
        int max_open_file = DEFAULT_MAX_OPEN_FILE,
        size_t default_cache_size = DEFAULT_CACHE_SIZE
    );
    ~BCaterpillarNimCalculator();

    BCaterpillarNimFileManager* get_file_manager() const;
    unsigned int calculate_nim(const Caterpillar *c, const VerboseClass &verb = VerboseClass(false)) override;

    void calculate_until(
        unsigned int x_class,
        const std::function<bool(int, std::chrono::milliseconds)> &stop_condition,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        const VerboseClass &verb = VerboseClass(false)
    );

    void calculate_by_n(
        unsigned int x_class,
        unsigned int n_limit,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        const VerboseClass &verb = VerboseClass(false)
    );

    void calculate_by_time(
        unsigned int x_class,
        const std::chrono::milliseconds &time_limit,
        const std::chrono::milliseconds &display_interval = std::chrono::minutes(1),
        const VerboseClass &verb = VerboseClass(false)
    );
};

#endif // KAYLES_BCATERPILLAR_H

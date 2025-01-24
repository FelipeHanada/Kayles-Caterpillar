#include "kayles_bcaterpillar.h"


BCaterpillar::BCaterpillar(int n, unsigned int x)
: Caterpillar(n) {
    int j = std::min(n - 2, (int) sizeof(x) * 8);
    this->x_class = 0;
    for (int i = 0; i < j; i++) {
        this->x[i + 1] = (x & (1 << i)) ? 1 : 0;
        this->x_class += (this->x[i + 1]) ? (1 << i) : 0;
    }
}

BCaterpillar::BCaterpillar(int n)
: Caterpillar(n) {
    this->x_class = 0;
}

BCaterpillar::BCaterpillar(std::vector<int> x)
: Caterpillar(x) {
    this->x_class = 0;
    for (int i=1; i<x.size() - 1; i++) {
        if (x[i]) {
            x[i] = 1;
            this->x_class += 1 << (i - 1);
        }
    }
}

BCaterpillar::BCaterpillar(const Caterpillar* c)
: Caterpillar(c->get_x()) {
    this->x_class = 0;
    x[0] = x[x.size() - 1] = 0;
    for (int i=1; i<x.size() - 1; i++) {
        if (x[i] % 2) {
            x[i] = 1;
            this->x_class += 1 << (i - 1);
        }
    }
}

BCaterpillar::~BCaterpillar() {}

unsigned int BCaterpillar::get_x_class() const {
    return x_class;
}

Caterpillar* BCaterpillarFactory::create(int n) {
    return new BCaterpillar(n);
}

Caterpillar* BCaterpillarFactory::create(std::vector<int> x) {
    return new BCaterpillar(x);
}

BCaterpillarNimFile::BCaterpillarNimFile(
    std::string filename,
    unsigned int x_class,
    size_t cache_size,
    bool start_open,
    bool verbose
) {
    this->filename = filename;
    this->x_class = x_class;
    this->offset = (x_class == 0) ? 0 : 3 + log2(x_class);
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

BCaterpillarNimFile::~BCaterpillarNimFile() {
    if (is_open())
        close();
    delete verb;
}

void BCaterpillarNimFile::open() {
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

bool BCaterpillarNimFile::is_open() {
    return file.is_open();
}

void BCaterpillarNimFile::close() {
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
    
size_t BCaterpillarNimFile::size() {
    return file_size / sizeof(unsigned int);
}

size_t BCaterpillarNimFile::get_pos(unsigned int n) {
    if (n < offset)
        return 0;
    return n - offset;
}

int BCaterpillarNimFile::get_n(size_t pos) {
    return pos + offset;
}

unsigned int BCaterpillarNimFile::read(size_t pos) {
    if (!this->is_open()) {
        std::cerr << "File not open" << "\n";
        exit(EXIT_FAILURE);
    }

    unsigned int nim;
    file.seekg(pos * sizeof(unsigned int));
    file.read((char*)&nim, sizeof(unsigned int));
    return nim;
}

void BCaterpillarNimFile::write(size_t pos, unsigned int nim) {
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

void BCaterpillarNimFile::write_n(unsigned int n, unsigned int nim) {
    write(get_pos(n), nim);
}

unsigned int BCaterpillarNimFile::read_cached(size_t pos) {
    if (pos < this->cache_size)
        return this->cache[pos];

    return read(pos);
}

unsigned int BCaterpillarNimFile::read_n(unsigned int n) {
    return read_cached(get_pos(n));
}

unsigned int BCaterpillarNimFile::is_calculated(unsigned int n) {
    return get_pos(n) < this->size();
}

bool BCaterpillarNimFile::is_cached(unsigned int n) {
    return get_pos(n) < this->cache_size;
}

void BCaterpillarNimFile::write(unsigned int nim) {
    this->write(this->size(), nim);
}

BCaterpillarNimFileManager::BCaterpillarNimFileManager(
    std::string file_prefix,
    int max_open_file,
    size_t default_cache_size,
    bool verbose,
    bool file_verbose
) {
    this->file_prefix = file_prefix;
    this->max_open_file = max_open_file;
    this->default_cache_size = default_cache_size;
    this->verb = new VerboseClass(verbose);
    this->file_verbose = file_verbose;
}

BCaterpillarNimFileManager::~BCaterpillarNimFileManager() {
    for (auto file : files)
        delete file;
    delete verb;
}

void BCaterpillarNimFileManager::open_file(unsigned int x) {
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

BCaterpillarNimFile* BCaterpillarNimFileManager::get_file(unsigned int x) {
    for (unsigned int i=files.size(); i <= x; i++)
        files.push_back(new BCaterpillarNimFile(
            file_prefix + (std::to_string(i)),
            i,
            default_cache_size,
            false,
            file_verbose
        ));

    return files[x];
}

void BCaterpillarNimFileManager::close() {
    while (!open_files.empty()) {
        unsigned int x = open_files_queue.front();
        open_files_queue.pop();
        files[x]->close();
    }

    open_files.clear();
}

BCaterpillarNimCalculator::BCaterpillarNimCalculator(
    std::string file_prefix, 
    int max_open_file,
    size_t default_cache_size
) : CaterpillarNimCalculator(new BCaterpillarFactory())
{
    this->file_manager = new BCaterpillarNimFileManager(file_prefix, max_open_file, default_cache_size);

    BCaterpillarNimFile *file0 = file_manager->get_file(0);
    file_manager->open_file(0);
    file0->write(0, 0);
    file0->write(1, 1);
}

BCaterpillarNimCalculator::~BCaterpillarNimCalculator() {
    delete file_manager;
}

BCaterpillarNimFileManager* BCaterpillarNimCalculator::get_file_manager() const {
    return file_manager;
}

unsigned int BCaterpillarNimCalculator::calculate_nim(const Caterpillar *c, const VerboseClass &verb) {
    const BCaterpillar *bc = new BCaterpillar(c);
    
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

    unsigned int nim = CaterpillarNimCalculator::calculate_nim(c, verb);

    file_manager->open_file(bc->get_x_class());
    file->write(nim);
    return nim;
}

void BCaterpillarNimCalculator::calculate_until(
    unsigned int x_class,
    const std::function<bool(int, std::chrono::milliseconds)> &stop_condition,
    const std::chrono::milliseconds &display_interval,
    const VerboseClass &verb
) {
    auto start = std::chrono::high_resolution_clock::now();
    auto next_display_time = start + display_interval;

    BCaterpillarNimFileManager *file_manager = get_file_manager();
    BCaterpillarNimFile *file = file_manager->get_file(x_class);

    int n = file->get_n(file->size());
    
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

        Caterpillar *c = new BCaterpillar(n, x_class);
        calculate_nim(c);
        delete c;
        n++;

        now = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
    }

    ss << "End of calculations. Last calculated n=" << n - 1 << "\n";
    verb.print(ss.str());
}

void BCaterpillarNimCalculator::calculate_by_n(
    unsigned int x_class,
    unsigned int n_limit,
    const std::chrono::milliseconds &display_interval,
    const VerboseClass &verb
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

void BCaterpillarNimCalculator::calculate_by_time(
    unsigned int x_class,
    const std::chrono::milliseconds &time_limit,
    const std::chrono::milliseconds &display_interval,
    const VerboseClass &verb
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

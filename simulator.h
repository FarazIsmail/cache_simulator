#pragma once

#include <wx/string.h>
#include <string>
#include <vector>

struct Line { bool valid = false; unsigned long long tag = 0; long used = -1, born = -1; };
enum class Policy { LRU, FIFO, RANDOM };
struct Config { int total = 0, block = 0, assoc = 0, sets = 0, off = 0, idx = 0; };
struct Stats { int hits = 0, misses = 0, evictions = 0; };
struct Trace { char op; unsigned long long addr; };
struct Result { bool hit = false, evicted = false; int set = -1, way = -1; std::string text; };

class Cache {
public:
    Config cfg;
    Stats stats;
    Policy policy = Policy::LRU;
    std::vector<std::vector<Line>> set;
    long tick = 0;

    bool ready() const;
    bool reset(int total, int block, int assoc, Policy p, wxString& err);
    Result access(Trace t);

private:
    static bool fail(wxString& err, const wxString& msg);
    int victim(const std::vector<Line>& ways);
};

std::vector<Trace> parseTrace(const wxString& text);

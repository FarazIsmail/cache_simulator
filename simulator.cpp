#include "simulator.h"
#include <algorithm>
#include <cctype>
#include <random>
#include <sstream>

static bool pow2(int x) { return x > 0 && !(x & (x - 1)); }
static int lg2(int x) { int n = 0; while (x > 1) x >>= 1, n++; return n; }

bool Cache::ready() const {
    return !set.empty();
}

bool Cache::reset(int total, int block, int assoc, Policy p, wxString& err) {
    if (total <= 0 || block <= 0 || assoc <= 0) return fail(err, "All cache values must be positive.");
    if (!pow2(total) || !pow2(block) || !pow2(assoc)) return fail(err, "All cache values must be powers of 2.");
    if (block > total) return fail(err, "Block size cannot be larger than total size.");
    int lines = total / block;
    if (assoc > lines) return fail(err, "Associativity cannot be larger than total lines.");
    cfg = {total, block, assoc, lines / assoc, lg2(block), lg2(lines / assoc)};
    stats = {};
    policy = p;
    tick = 0;
    set.assign(cfg.sets, std::vector<Line>(assoc));
    return true;
}

Result Cache::access(Trace t) {
    unsigned long long tag = t.addr >> (cfg.off + cfg.idx);
    int idx = int((t.addr >> cfg.off) & ((1ULL << cfg.idx) - 1));
    auto& ways = set[idx];
    Result r;
    r.set = idx;
    tick++;

    for (int i = 0; i < (int)ways.size(); i++)
        if (ways[i].valid && ways[i].tag == tag) {
            ways[i].used = tick;
            stats.hits++;
            r.hit = true;
            r.way = i;
            break;
        }

    if (!r.hit) {
        stats.misses++;
        r.way = victim(ways);
        r.evicted = ways[r.way].valid;
        stats.evictions += r.evicted;
        ways[r.way] = {true, tag, tick, tick};
    }

    std::ostringstream s;
    s << "Address 0x" << std::hex << t.addr << " [" << t.op << "] -> Set " << std::dec << idx
      << ", Tag 0x" << std::hex << tag << " -> " << (r.hit ? "HIT" : "MISS");
    if (!r.hit) s << (r.evicted ? " (Evicted Line " : " (Loaded Line ") << std::dec << r.way << ")";
    r.text = s.str();
    return r;
}

bool Cache::fail(wxString& err, const wxString& msg) {
    err = msg;
    return false;
}

int Cache::victim(const std::vector<Line>& ways) {
    for (int i = 0; i < (int)ways.size(); i++) if (!ways[i].valid) return i;
    if (policy == Policy::RANDOM) {
        static std::mt19937 gen{std::random_device{}()};
        return std::uniform_int_distribution<int>(0, (int)ways.size() - 1)(gen);
    }
    auto field = policy == Policy::LRU ? &Line::used : &Line::born;
    return int(std::min_element(ways.begin(), ways.end(), [&](auto& a, auto& b) { return a.*field < b.*field; }) - ways.begin());
}

std::vector<Trace> parseTrace(const wxString& text) {
    std::vector<Trace> out;
    std::istringstream in(text.ToStdString());
    for (std::string line, addr; std::getline(in, line);) {
        std::istringstream row(line);
        char op;
        if (row >> op >> addr) {
            op = (char)std::toupper((unsigned char)op);
            if (op == 'R' || op == 'W') try { out.push_back({op, std::stoull(addr, nullptr, 16)}); } catch (...) {}
        }
    }
    return out;
}

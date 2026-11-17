// =============================================================================
//  Question 2 : Cache Mapping Simulation
//  Compares Direct-Mapped / Set-Associative / Fully Associative mapping.
//
//  Model
//  -----
//  * The cache holds `blocks` cache lines (default 4, as in the question).
//  * Every memory request is a block number, so block size = 1 block.
//  * All three techniques are simulated by the SAME engine:
//        - Direct-Mapped      -> sets = blocks, ways = 1
//        - Set-Associative    -> sets = blocks / ways  (ways >= 2)
//        - Fully Associative  -> sets = 1, ways = blocks
//    The only difference is how many sets exist, which is exactly what the
//    mapping technique decides.  Replacement inside a set uses LRU.
//
//  Mapping rule :  set = (block number) mod (number of sets)
//
//  Build : g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o cache_sim
//  Run   : ./cache_sim                          (question's example)
//          ./cache_sim 8 0 4 8 12 16 20 24      (8 blocks, custom trace)
//          ./cache_sim 8 -a 4 0 1 2 3 4 5 6     (8 blocks, 4-way)
// =============================================================================

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
//  One cache line
// -----------------------------------------------------------------------------
struct Line {
    bool      valid    = false;
    long long block    = -1;   // which memory block is stored here
    long long lastUsed = 0;    // counter used by the LRU policy
};

// -----------------------------------------------------------------------------
//  The cache itself (one engine for all three mappings)
// -----------------------------------------------------------------------------
class Cache {
public:
    Cache(int blocks, int ways)
        : ways_(ways), sets_(blocks / ways), grid_(sets_, std::vector<Line>(ways)) {}

    int ways() const { return ways_; }
    int sets() const { return sets_; }
    const std::vector<std::vector<Line>>& grid() const { return grid_; }

    // What happened to one memory request
    struct Outcome {
        bool      hit     = false;
        int       set     = 0;
        int       way     = 0;
        long long evicted = -1;    // block thrown out (-1 = nothing evicted)
    };

    Outcome access(long long block, long long tick) {
        const int set = static_cast<int>(((block % sets_) + sets_) % sets_);
        auto& lines = grid_[set];

        // ---- probe every way of the selected set -------------------------
        for (int w = 0; w < ways_; ++w) {
            if (lines[w].valid && lines[w].block == block) {
                lines[w].lastUsed = tick;          // refresh LRU age
                return {true, set, w, -1};
            }
        }

        // ---- miss: choose a victim ---------------------------------------
        int       way     = -1;
        long long evicted = -1;

        for (int w = 0; w < ways_; ++w) {          // prefer an empty line
            if (!lines[w].valid) { way = w; break; }
        }
        if (way < 0) {                             // set is full -> LRU victim
            way = 0;
            for (int w = 1; w < ways_; ++w)
                if (lines[w].lastUsed < lines[way].lastUsed) way = w;
            evicted = lines[way].block;
        }

        lines[way] = Line{true, block, tick};
        return {false, set, way, evicted};
    }

private:
    int                             ways_;
    int                             sets_;
    std::vector<std::vector<Line>>  grid_;
};

// -----------------------------------------------------------------------------
//  Small formatting helpers
// -----------------------------------------------------------------------------
static std::string formatSet(int index, const std::vector<Line>& s) {
    std::ostringstream os;
    os << "S" << index << "[";
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (i) os << ' ';
        os << (s[i].valid ? std::to_string(s[i].block) : std::string("-"));
    }
    os << "]";
    return os.str();
}

static std::string formatContents(const Cache& c) {
    std::string out;
    for (int i = 0; i < c.sets(); ++i) {
        if (i) out += ' ';
        out += formatSet(i, c.grid()[i]);
    }
    return out;
}

static std::string pct(long long part, long long whole) {
    if (whole <= 0) return "  0.00%";
    std::ostringstream os;
    os << std::fixed << std::setprecision(2)
       << (100.0 * static_cast<double>(part) / static_cast<double>(whole)) << "%";
    return os.str();
}

struct Stats {
    std::string name;
    std::string shortName;
    int         sets = 0;
    int         ways = 0;
    long long   hits = 0;
    long long   misses = 0;
    long long   evictions = 0;
    long long   total() const { return hits + misses; }
};

// -----------------------------------------------------------------------------
//  Run one technique and print its per-request table
// -----------------------------------------------------------------------------
static Stats simulate(const std::string&            title,
                      const std::string&            shortName,
                      const std::string&            formula,
                      int                           blocks,
                      int                           ways,
                      const std::vector<long long>& requests)
{
    Cache cache(blocks, ways);
    Stats st;
    st.name      = title;
    st.shortName = shortName;
    st.sets = cache.sets();
    st.ways = cache.ways();

    std::cout << "\n" << std::string(96, '-') << "\n";
    std::cout << title << "\n";
    std::cout << "  Cache layout : " << blocks << " blocks = "
              << cache.sets() << " set(s) x " << cache.ways() << " way(s)   |   "
              << formula << "\n";
    std::cout << std::string(96, '-') << "\n";
    std::cout << std::left
              << std::setw(4)  << "#"
              << std::setw(9)  << "Request"
              << std::setw(7)  << "Set"
              << std::setw(7)  << "Way"
              << std::setw(8)  << "Result"
              << std::setw(10) << "Evicted"
              << "Cache contents after the access\n";

    long long tick = 1;
    for (std::size_t i = 0; i < requests.size(); ++i) {
        const auto r = cache.access(requests[i], tick++);
        if (r.hit) ++st.hits; else ++st.misses;
        if (r.evicted >= 0) ++st.evictions;

        std::cout << std::left
                  << std::setw(4)  << (i + 1)
                  << std::setw(9)  << requests[i]
                  << std::setw(7)  << ("S" + std::to_string(r.set))
                  << std::setw(7)  << ("W" + std::to_string(r.way))
                  << std::setw(8)  << (r.hit ? "HIT" : "MISS")
                  << std::setw(10) << (r.evicted >= 0 ? std::to_string(r.evicted) : std::string("-"))
                  << formatContents(cache) << "\n";
    }

    std::cout << std::string(96, '-') << "\n";
    std::cout << "  Hits = " << st.hits << "   Misses = " << st.misses
              << "   Hit rate = " << pct(st.hits, st.total())
              << "   Miss rate = " << pct(st.misses, st.total()) << "\n";
    std::cout << "  Final state: " << formatContents(cache) << "\n";
    return st;
}

// -----------------------------------------------------------------------------
//  Comparison table + written conclusion
// -----------------------------------------------------------------------------
static void report(const std::vector<Stats>& all, int blocks) {
    std::cout << "\n" << std::string(96, '=') << "\n";
    std::cout << "COMPARISON SUMMARY   (" << blocks << " cache blocks, trace of "
              << all.front().total() << " requests)\n";
    std::cout << std::string(96, '=') << "\n";
    std::cout << std::left
              << std::setw(26) << "Technique"
              << std::setw(13) << "Sets x Ways"
              << std::setw(8)  << "Hits"
              << std::setw(9)  << "Misses"
              << std::setw(11) << "Hit rate"
              << std::setw(11) << "Miss rate"
              << "Evictions\n";

    for (const auto& s : all) {
        std::ostringstream layout;
        layout << s.sets << " x " << s.ways;
        std::cout << std::left
                  << std::setw(26) << s.shortName
                  << std::setw(13) << layout.str()
                  << std::setw(8)  << s.hits
                  << std::setw(9)  << s.misses
                  << std::setw(11) << pct(s.hits, s.total())
                  << std::setw(11) << pct(s.misses, s.total())
                  << s.evictions << "\n";
    }

    // -------- pick the winner (more hits, then fewer evictions) ------------
    const Stats* best = &all.front();
    for (const auto& s : all) {
        if (s.hits > best->hits ||
            (s.hits == best->hits && s.evictions < best->evictions))
            best = &s;
    }

    std::cout << std::string(96, '=') << "\n";
    std::cout << "BEST FOR THIS TRACE : " << best->name
              << "  (" << best->hits << " hits / " << best->total() << " requests)\n";
    std::cout << std::string(96, '=') << "\n";

    std::cout << R"(
WHY (brief explanation)
-----------------------
* Direct-mapped keeps exactly ONE line per set and the set is fixed by
  (block mod sets).  Two blocks that map to the same set fight for that single
  line, so the later request always throws the earlier one out.  This is a
  CONFLICT MISS: the data may still be needed, but there is nowhere to keep it.

* Full associativity lets a block live in ANY line of the cache, so the only
  misses left are COMPULSORY (first touch) and CAPACITY misses.  It needs the
  most comparators/hardware to search all ways, which is why real CPUs use it
  only for small structures such as TLBs.

* Set-associative mapping is the compromise: a block may go into any way of its
  set, so a couple of colliding blocks can coexist, but the hardware only has to
  compare `ways` tags instead of all `blocks` tags.

In the example trace 0, 4, 8, 0, 4, 12 all blocks are multiples of 4, so every
one of them maps to set 0 in the direct-mapped cache (mod 4) and also to set 0 in
the 2-way cache (mod 2).  All six requests therefore pile onto one set and the
caches thrash: 0 hits.  The fully associative cache spreads those same blocks
over all 4 lines, so the repeats of 0 and 4 are served from the cache: 2 hits.
=> For this access sequence the FULLY ASSOCIATIVE mapping gives the best
   flexibility and the lowest miss rate; direct-mapped is the most rigid.
)";
}

// -----------------------------------------------------------------------------
//  main
// -----------------------------------------------------------------------------
static void usage(const char* exe) {
    std::cout <<
        "Usage: " << exe << " [cacheBlocks] [-a ways] [block block ...]\n\n"
        "  cacheBlocks : number of cache lines (default 4)\n"
        "  -a ways     : associativity for the set-associative run (default 2)\n"
        "  blocks...   : memory block requests (default 0 4 8 0 4 12)\n\n"
        "Examples:\n"
        "  " << exe << "\n"
        "  " << exe << " 8 0 4 8 12 16 20 24\n"
        "  " << exe << " 8 -a 4 0 1 2 3 4 5 6\n";
}

int main(int argc, char** argv) {
    int blocks = 4;
    int assocWays = 2;
    std::vector<long long> requests = {0, 4, 8, 0, 4, 12};

    std::vector<std::string> args(argv + 1, argv + argc);
    if (!args.empty() && (args[0] == "-h" || args[0] == "--help")) {
        usage(argv[0]);
        return 0;
    }

    // ---- parse ------------------------------------------------------------
    std::size_t i = 0;
    if (i < args.size()) {
        try {
            std::size_t used = 0;
            int v = std::stoi(args[i], &used);
            if (used == args[i].size() && v > 0) { blocks = v; ++i; }
        } catch (...) { /* not a number: treat everything as a request */ }
    }
    if (i + 1 < args.size() && args[i] == "-a") {
        try { assocWays = std::max(2, std::stoi(args[i + 1])); i += 2; }
        catch (...) { std::cerr << "Bad -a value\n"; return 1; }
    }
    if (i < args.size()) {
        requests.clear();
        for (; i < args.size(); ++i) {
            try { requests.push_back(std::stoll(args[i])); }
            catch (...) {
                std::cerr << "Error: '" << args[i] << "' is not a memory block number.\n";
                return 1;
            }
        }
    }
    if (requests.empty()) {
        std::cerr << "Error: need at least one memory block request.\n";
        return 1;
    }

    // The set-associative count must split the cache into equal sets, so fall
    // back to the largest associativity that divides the number of blocks.
    if (assocWays >= blocks) assocWays = std::max(1, blocks / 2);
    while (assocWays > 1 && blocks % assocWays != 0) --assocWays;
    const bool assocDegraded = (assocWays < 2);
    if (assocDegraded) assocWays = 1;

    // ---- banner -----------------------------------------------------------
    std::cout << std::string(96, '=') << "\n";
    std::cout << " QUESTION 2 : CACHE MAPPING SIMULATION\n";
    std::cout << " Direct-Mapped  vs  Set-Associative  vs  Fully Associative\n";
    std::cout << std::string(96, '=') << "\n";
    std::cout << " Cache blocks    : " << blocks << "   (block size = 1 block, so 1 request = 1 block)\n";
    std::cout << " Memory requests : ";
    for (std::size_t k = 0; k < requests.size(); ++k)
        std::cout << (k ? ", " : "") << requests[k];
    std::cout << "\n";
    std::cout << " Replacement     : LRU inside each set\n";
    std::cout << " Mapping rule    : set = (block number) mod (number of sets)\n";
    if (assocDegraded)
        std::cout << " [note] " << blocks << " blocks cannot be split into 2 or more equal"
                     " sets, so the set-associative run uses 1 way.\n";

    // ---- the three techniques --------------------------------------------
    Stats dm = simulate("1) DIRECT-MAPPED CACHE   (" + std::to_string(blocks) + " sets x 1 way)",
                        "1) Direct-Mapped",
                        "set = block mod " + std::to_string(blocks),
                        blocks, 1, requests);

    Stats sa = simulate("2) " + std::to_string(assocWays) + "-WAY SET-ASSOCIATIVE CACHE   ("
                            + std::to_string(blocks / assocWays) + " sets x "
                            + std::to_string(assocWays) + " ways)",
                        "2) " + std::to_string(assocWays) + "-Way Set-Associative",
                        "set = block mod " + std::to_string(blocks / assocWays),
                        blocks, assocWays, requests);

    Stats fa = simulate("3) FULLY ASSOCIATIVE CACHE   (1 set x " + std::to_string(blocks) + " ways)",
                        "3) Fully Associative",
                        "set = block mod 1  (any line can hold any block)",
                        blocks, blocks, requests);

    report({dm, sa, fa}, blocks);
    return 0;
}

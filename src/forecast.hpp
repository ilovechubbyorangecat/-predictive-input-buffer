// Attempt Progression & Forecasting: the model.
//
// Pure C++ (no game code), so it can be compiled and tested on its own.
//
// You say at which attempt the level is beaten (the target). Every attempt before it then has
// a percentage it ends at, worked out from the attempt's number alone: nothing is generated
// ahead of time, so a target of 500,000 costs the same as one of 5, and any attempt can be
// asked for directly.
//
//   reach(k)   how far the run "can" get by attempt k: a curve from the start of the run to 100%
//              at the target. Checkpoints you lock (attempt 200 = 60%) are points the curve goes
//              through, with straight stretches between them.
//   outcome(k) where attempt k dies: at a checkpoint exactly its percentage; at the target 100%;
//              otherwise somewhere below reach(k), chosen by a hash of (seed, start, k). The fewer
//              attempts the target leaves, the closer every attempt is to reach(k) (5 attempts:
//              each one a clear step forward); a long target dies early most of the time, with
//              now and then a run up to its best.
//
// A run from a start position has its own count and its own, shorter target (the part of the
// level it covers, at practice speed). When it reaches 100% that start is "practiced": a later
// run from before it that gets to it clears the rest most of the time.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace forecast {
    constexpr float CURVE = 0.7f;            // the shape of the first stretch (below 1: quick first gains)
    constexpr float REACH_FLOOR = 3.f;       // nobody's best is below the first obstacles
    constexpr float PRACTICE_SPEED = 0.5f;   // a part practiced alone takes this much of its share of the target
    constexpr float MASTERY_CLEAR = 0.9f;    // chance a run that reaches a practiced start clears the rest
    constexpr float BEATEN_CLEAR = 0.6f;     // after the level is beaten: chance an attempt beats it again
    constexpr int64_t TARGET_MAX = 100000000;

    struct Checkpoint {
        int64_t attempt = 0;
        float percent = 0.f;
    };

    // runs from one start position
    struct Start {
        float from = 0.f;       // %
        int64_t attempts = 0;   // attempts done from here
        int completions = 0;    // of them, to the end (1 or more: practiced)
    };

    struct Memory {
        int64_t target = 100;       // the attempt (from 0%) the level is beaten at
        int64_t attempts = 0;       // attempts done from 0%
        int completions = 0;        // of them, to the end
        float best = 0.f;           // best from 0% (%)
        uint64_t seed = 1;
        std::vector<Checkpoint> checkpoints;  // sorted by attempt
        std::vector<Start> starts;            // sorted by from

        bool beaten() const {
            return completions > 0;
        }
    };

    inline uint64_t mix(uint64_t x) {
        x += 0x9E3779B97F4A7C15ull;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        return x ^ (x >> 31);
    }

    // 0 <= x < 1, the same for the same (seed, start, attempt, which)
    inline float unit(uint64_t seed, float start, int64_t attempt, int which) {
        uint64_t h = mix(seed ^ mix(static_cast<uint64_t>(attempt) * 0x100000001B3ull + static_cast<uint64_t>(std::lround(start * 10.f)) * 7919ull +
                                    static_cast<uint64_t>(which) * 104729ull));
        return static_cast<float>(h >> 40) / 16777216.f;
    }

    inline bool fromStart(float start) {
        return start < 0.5f;
    }

    // keeps the list valid: in the target, in 0..100, one per attempt, sorted
    inline void normalize(Memory& m) {
        m.target = std::clamp<int64_t>(m.target, 1, TARGET_MAX);
        m.attempts = std::max<int64_t>(m.attempts, 0);
        std::vector<Checkpoint> ok;
        for (auto c : m.checkpoints) {
            c.percent = std::clamp(c.percent, 0.f, 100.f);
            if (c.attempt >= 1 && c.attempt <= TARGET_MAX) {
                ok.push_back(c);
            }
        }
        std::stable_sort(ok.begin(), ok.end(), [](Checkpoint const& a, Checkpoint const& b) { return a.attempt < b.attempt; });
        m.checkpoints.clear();
        for (auto const& c : ok) {
            if (!m.checkpoints.empty() && m.checkpoints.back().attempt == c.attempt) {
                m.checkpoints.back() = c;  // the same attempt twice: the later one counts
            }
            else {
                m.checkpoints.push_back(c);
            }
        }
    }

    // the target of runs from `start`: the whole target from 0%, the part's share from further in
    inline int64_t targetFor(Memory const& m, float start) {
        if (fromStart(start)) {
            return m.target;
        }
        double share = static_cast<double>(m.target) * (100.0 - start) / 100.0 * PRACTICE_SPEED;
        return std::max<int64_t>(1, static_cast<int64_t>(std::ceil(share)));
    }

    inline Start const* findStart(Memory const& m, float start) {
        for (auto const& s : m.starts) {
            if (std::abs(s.from - start) < 0.75f) {
                return &s;
            }
        }
        return nullptr;
    }

    // attempts done from `start`
    inline int64_t attemptsFrom(Memory const& m, float start) {
        if (fromStart(start)) {
            return m.attempts;
        }
        auto* s = findStart(m, start);
        return s ? s->attempts : 0;
    }

    inline bool beatenFrom(Memory const& m, float start) {
        if (fromStart(start)) {
            return m.beaten();
        }
        auto* s = findStart(m, start);
        return s && s->completions > 0;
    }

    // the earliest practiced start after `start` (100 or more: none)
    inline float practicedAfter(Memory const& m, float start) {
        float best = 1000.f;
        for (auto const& s : m.starts) {
            if (s.completions > 0 && s.from > start + 0.75f) {
                best = std::min(best, s.from);
            }
        }
        return best;
    }

    // how far a run from `start` can get by its attempt k (see the top of the file)
    inline float reach(Memory const& m, float start, int64_t k) {
        int64_t target = targetFor(m, start);
        if (k >= target) {
            return 100.f;
        }
        // the points the curve goes through: (0, start), the checkpoints (runs from 0% only), (target, 100)
        int64_t a0 = 0, a1 = target;
        float p0 = start, p1 = 100.f;
        bool first = true;
        if (fromStart(start)) {
            for (auto const& c : m.checkpoints) {
                if (c.attempt >= target) {
                    break;
                }
                if (c.attempt <= k) {
                    a0 = c.attempt;
                    p0 = c.percent;
                    first = false;
                }
                else {
                    a1 = c.attempt;
                    p1 = c.percent;
                    break;
                }
            }
        }
        double u = static_cast<double>(k - a0) / static_cast<double>(std::max<int64_t>(1, a1 - a0));
        double shaped = first ? std::pow(u, static_cast<double>(CURVE)) : u;
        float value = p0 + (p1 - p0) * static_cast<float>(shaped);
        float floor = std::min(100.f, start + REACH_FLOOR);
        return std::clamp(std::max(value, std::min(floor, std::max(p0, p1))), start, 100.f);
    }

    // how close to reach() the attempts are: 12 = nearly always right at it, 0.35 = mostly early deaths
    inline float consistency(int64_t target) {
        return std::clamp(30.f / std::pow(static_cast<float>(std::max<int64_t>(1, target)), 0.6f), 0.35f, 12.f);
    }

    // Where attempt k (1 = the first) of the runs from `start` ends, in %. 100: it beats the level.
    inline float outcome(Memory const& m, float start, int64_t k) {
        start = std::clamp(start, 0.f, 99.f);
        k = std::max<int64_t>(k, 1);
        int64_t target = targetFor(m, start);
        bool zero = fromStart(start);
        if (zero) {
            for (auto const& c : m.checkpoints) {
                if (c.attempt == k && k < target) {
                    return c.percent;  // locked
                }
            }
        }
        if (!beatenFrom(m, start)) {
            if (k >= target) {
                return 100.f;  // the target (and every attempt after it until it is really beaten)
            }
        }
        else {
            // beaten already: it knows the way, and still chokes
            if (unit(m.seed, start, k, 2) < BEATEN_CLEAR) {
                return 100.f;
            }
            float u = unit(m.seed, start, k, 0);
            return std::min(99.4f, start + (100.f - start) * (1.f - std::pow(u, 3.f)));
        }
        float top = reach(m, start, k);
        float u = unit(m.seed, start, k, 0);
        float value = start + (top - start) * (1.f - std::pow(u, consistency(target)));
        // never right on the start, never a completion by accident
        value = std::clamp(value, std::min(start + 0.4f, top), std::min(top, 99.4f));
        // a practiced part ahead: getting to it mostly means getting through it
        float practiced = practicedAfter(m, start);
        if (value >= practiced && unit(m.seed, start, k, 1) < MASTERY_CLEAR) {
            return 100.f;
        }
        return value;
    }

    // the next attempt's number and outcome for runs from `start`
    inline int64_t nextAttempt(Memory const& m, float start) {
        return attemptsFrom(m, start) + 1;
    }

    // attempts left until the target for runs from `start` (0: this is the one, or it is beaten)
    inline int64_t remaining(Memory const& m, float start) {
        if (beatenFrom(m, start)) {
            return 0;
        }
        return std::max<int64_t>(0, targetFor(m, start) - attemptsFrom(m, start));
    }

    // an attempt from `start` ended at `end` (%); `finished`: it reached the end of the level
    inline void record(Memory& m, float start, float end, bool finished) {
        if (fromStart(start)) {
            ++m.attempts;
            m.best = std::max(m.best, finished ? 100.f : end);
            if (finished) {
                ++m.completions;
            }
            return;
        }
        Start* s = nullptr;
        for (auto& x : m.starts) {
            if (std::abs(x.from - start) < 0.75f) {
                s = &x;
            }
        }
        if (!s) {
            m.starts.push_back({ start, 0, 0 });
            std::sort(m.starts.begin(), m.starts.end(), [](Start const& a, Start const& b) { return a.from < b.from; });
            for (auto& x : m.starts) {
                if (std::abs(x.from - start) < 0.75f) {
                    s = &x;
                }
            }
        }
        ++s->attempts;
        if (finished) {
            ++s->completions;
        }
    }

    // "200:60, 500:87, 762:97" (also "200=60%", ";" or new lines between them)
    inline std::vector<Checkpoint> parseCheckpoints(std::string const& text) {
        std::vector<Checkpoint> out;
        std::string token;
        auto flush = [&] {
            size_t sep = token.find_first_of(":=");
            if (sep != std::string::npos) {
                try {
                    Checkpoint c;
                    c.attempt = std::stoll(token.substr(0, sep));
                    c.percent = std::stof(token.substr(sep + 1));
                    out.push_back(c);
                }
                catch (...) {
                }
            }
            token.clear();
        };
        for (char ch : text) {
            if (ch == ',' || ch == ';' || ch == '\n') {
                flush();
            }
            else if (ch != ' ' && ch != '%') {
                token += ch;
            }
        }
        flush();
        return out;
    }

    inline std::string percentText(float p) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), std::abs(p - std::round(p)) < 0.05f ? "%.0f" : "%.1f", p);
        return buf;
    }

    inline std::string checkpointsText(Memory const& m) {
        std::string out;
        for (auto const& c : m.checkpoints) {
            if (!out.empty()) {
                out += ", ";
            }
            out += std::to_string(c.attempt) + ":" + percentText(c.percent);
        }
        return out;
    }

    // "88-100% x2, 62-100%"
    inline std::string practicedText(Memory const& m, size_t most = 4) {
        std::string out;
        size_t shown = 0, total = 0;
        for (auto const& s : m.starts) {
            total += s.completions > 0 ? 1 : 0;
        }
        for (auto const& s : m.starts) {
            if (s.completions <= 0) {
                continue;
            }
            if (shown == most) {
                out += " +" + std::to_string(total - shown) + " more";
                break;
            }
            if (!out.empty()) {
                out += ", ";
            }
            out += std::to_string(static_cast<int>(std::lround(s.from))) + "-100%";
            if (s.completions > 1) {
                out += " x" + std::to_string(s.completions);
            }
            ++shown;
        }
        return out;
    }

    inline std::string serialize(Memory const& m) {
        std::ostringstream o;
        o << "forecast 1\n" << m.target << ' ' << m.attempts << ' ' << m.completions << ' ' << m.best << ' ' << m.seed << '\n';
        o << m.checkpoints.size() << '\n';
        for (auto const& c : m.checkpoints) {
            o << c.attempt << ' ' << c.percent << '\n';
        }
        o << m.starts.size() << '\n';
        for (auto const& s : m.starts) {
            o << s.from << ' ' << s.attempts << ' ' << s.completions << '\n';
        }
        return o.str();
    }

    inline bool deserialize(std::string const& text, Memory& out) {
        std::istringstream in(text);
        std::string tag;
        int version = 0;
        Memory m;
        if (!(in >> tag >> version) || tag != "forecast" || version != 1) {
            return false;
        }
        if (!(in >> m.target >> m.attempts >> m.completions >> m.best >> m.seed)) {
            return false;
        }
        size_t n = 0;
        if (!(in >> n) || n > 10000) {
            return false;
        }
        for (size_t i = 0; i < n; ++i) {
            Checkpoint c;
            if (!(in >> c.attempt >> c.percent)) {
                return false;
            }
            m.checkpoints.push_back(c);
        }
        if (!(in >> n) || n > 10000) {
            return false;
        }
        for (size_t i = 0; i < n; ++i) {
            Start s;
            if (!(in >> s.from >> s.attempts >> s.completions)) {
                return false;
            }
            m.starts.push_back(s);
        }
        normalize(m);
        out = m;
        return true;
    }
}

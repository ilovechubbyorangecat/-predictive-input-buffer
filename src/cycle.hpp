// The Cycle system's model: a bot that learns a level the way a player does.
//
// Pure C++ (no game code), so it can be compiled and tested on its own.
//
// The level is cut into 100 buckets of 1%. Each bucket has a skill from 0 (never seen) to 1
// (mastered). At the start of an attempt the bot rolls, bucket by bucket from where the attempt
// starts, whether it makes a mistake there; the first bucket that fails is where it will die.
//   chance to fail a bucket = choke + (noobFail - choke) * (1 - skill) ^ curve
// Afterwards every bucket it got through alive is practised a little (skill += rate * (1 - skill)),
// and the bucket it died in a little less. A run from a start position practises its buckets
// `practiceBoost` times as fast, and one that reaches the end lifts them to `retention` at least.
// Skill belongs to the bucket, not to the run, so what was practised from a start position is
// there when a run from 0% gets to it.
//
// `rate` isn't set by hand: the user gives a milestone ("15% after 32 attempts") and the rate is
// solved so that a beginner's best after that many attempts from 0% is that far (the median of a
// few dozen simulated beginners).
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace cycle {
    constexpr int BUCKETS = 100;

    struct Params {
        int attempts = 32;           // the milestone: after this many attempts from 0% ...
        float progress = 15.f;       // ... a beginner's best is this far (%)
        float noobFail = 0.60f;      // chance to die in 1% never seen before
        float choke = 0.006f;        // chance to die in 1% that is mastered
        float curve = 1.8f;          // > 1: stays bad for longer, then improves quickly
        float practiceBoost = 2.f;   // how much faster a start position run practises
        float retention = 0.65f;     // skill a finished start position run leaves at least
        float deathLearn = 0.5f;     // how much a death teaches, next to getting through

        bool operator==(Params const& o) const {
            return attempts == o.attempts && progress == o.progress && noobFail == o.noobFail && choke == o.choke && curve == o.curve &&
                practiceBoost == o.practiceBoost && retention == o.retention && deathLearn == o.deathLearn;
        }
    };

    struct Preset {
        char const* name;
        Params params;
    };

    constexpr int PRESET_COUNT = 5;
    inline Preset const PRESETS[PRESET_COUNT] = {
        //                  attempts progress noob   choke    curve boost retention death
        { "Rookie", { 60, 10.f, 0.70f, 0.0100f, 1.6f, 1.5f, 0.50f, 0.4f } },
        { "Casual", { 32, 15.f, 0.60f, 0.0060f, 1.8f, 2.0f, 0.65f, 0.5f } },
        { "Grinder", { 32, 25.f, 0.50f, 0.0040f, 2.0f, 2.5f, 0.78f, 0.5f } },
        { "Prodigy", { 30, 40.f, 0.40f, 0.0020f, 2.2f, 4.0f, 0.90f, 0.6f } },
        { "Demon Slayer", { 20, 60.f, 0.30f, 0.0008f, 2.5f, 8.0f, 0.97f, 0.7f } },
    };

    inline Params clamped(Params p) {
        p.attempts = std::clamp(p.attempts, 1, 5000);
        p.progress = std::clamp(p.progress, 1.f, 100.f);
        p.noobFail = std::clamp(p.noobFail, 0.02f, 0.98f);
        p.choke = std::clamp(p.choke, 0.f, p.noobFail);
        p.curve = std::clamp(p.curve, 0.3f, 6.f);
        p.practiceBoost = std::clamp(p.practiceBoost, 1.f, 20.f);
        p.retention = std::clamp(p.retention, 0.f, 1.f);
        p.deathLearn = std::clamp(p.deathLearn, 0.f, 1.f);
        return p;
    }

    // a small fast generator (the same numbers on every platform, so the solved rate is too)
    struct Rng {
        uint64_t s;
        explicit Rng(uint64_t seed) : s(seed ? seed : 0x9E3779B97F4A7C15ull) {}
        uint64_t next() {
            s ^= s << 13;
            s ^= s >> 7;
            s ^= s << 17;
            return s;
        }
        // 0 <= x < 1
        float unit() {
            return static_cast<float>(next() >> 40) / 16777216.f;
        }
    };

    // a start position run that reached the end
    struct Run {
        float from = 0.f;
        int count = 0;
    };

    struct Memory {
        int attempts = 0;        // every attempt, from anywhere
        int fromStart = 0;       // attempts from 0%
        int completions = 0;     // runs from 0% to the end
        float best = 0.f;        // best from 0% (%)
        std::array<float, BUCKETS> skill {};
        std::vector<Run> runs;

        float average() const {
            float sum = 0.f;
            for (float s : skill) {
                sum += s;
            }
            return sum / BUCKETS;
        }
    };

    inline float failChance(Params const& p, float skill) {
        return p.choke + (p.noobFail - p.choke) * std::pow(std::clamp(1.f - skill, 0.f, 1.f), p.curve);
    }

    inline int bucketOf(float percent) {
        return std::clamp(static_cast<int>(std::floor(percent)), 0, BUCKETS - 1);
    }

    // Where this attempt dies (%), starting at `start` (%). 100 or more: it makes no mistake.
    inline float planDeath(Params const& p, Memory const& m, float start, Rng& rng) {
        start = std::clamp(start, 0.f, 100.f);
        for (int b = bucketOf(start); b < BUCKETS; ++b) {
            float lo = std::max(start, static_cast<float>(b));
            float part = static_cast<float>(b + 1) - lo;  // the part of this bucket the run covers
            if (part <= 0.f) {
                continue;
            }
            if (rng.unit() < failChance(p, m.skill[b]) * part) {
                return lo + rng.unit() * part;
            }
        }
        return 1000.f;
    }

    // An attempt from `start` ended at `end` (%); `finished`: it reached the end of the level.
    inline void learn(Params const& p, Memory& m, float rate, float start, float end, bool finished) {
        start = std::clamp(start, 0.f, 100.f);
        end = finished ? 100.f : std::clamp(end, start, 100.f);
        bool practice = start >= 0.5f;
        float gain = std::min(1.f, rate * (practice ? p.practiceBoost : 1.f));
        int first = bucketOf(start);
        int last = finished ? BUCKETS : bucketOf(end);  // the bucket it died in (not passed)
        for (int b = first; b < last && b < BUCKETS; ++b) {
            m.skill[b] += gain * (1.f - m.skill[b]);
        }
        if (!finished && last < BUCKETS) {
            m.skill[last] += gain * p.deathLearn * (1.f - m.skill[last]);
        }
        ++m.attempts;
        if (!practice) {
            ++m.fromStart;
            m.best = std::max(m.best, end);
            if (finished) {
                ++m.completions;
            }
        }
        else if (finished) {
            for (int b = first; b < BUCKETS; ++b) {
                m.skill[b] = std::max(m.skill[b], p.retention);
            }
            bool found = false;
            for (auto& r : m.runs) {
                if (std::abs(r.from - start) < 0.75f) {
                    ++r.count;
                    found = true;
                    break;
                }
            }
            if (!found) {
                m.runs.push_back({ start, 1 });
                std::sort(m.runs.begin(), m.runs.end(), [](Run const& a, Run const& b) { return a.from < b.from; });
            }
        }
        for (float& s : m.skill) {
            s = std::clamp(s, 0.f, 1.f);
        }
    }

    // the median best (%) of simulated beginners after p.attempts attempts from 0%
    inline float medianBest(Params const& p, float rate) {
        // fewer beginners for long milestones, so solving stays quick (an odd number: a real median)
        int trials = std::clamp(15000 / std::max(1, p.attempts), 5, 31) | 1;
        std::vector<float> bests;
        Rng rng(0xC0FFEE1234ull);
        for (int t = 0; t < trials; ++t) {
            Memory m;
            for (int a = 0; a < p.attempts; ++a) {
                float death = planDeath(p, m, 0.f, rng);
                learn(p, m, rate, 0.f, death, death >= 100.f);
            }
            bests.push_back(m.best);
        }
        std::sort(bests.begin(), bests.end());
        return bests[bests.size() / 2];
    }

    // the learning rate that puts a beginner at p.progress after p.attempts attempts
    inline float solveRate(Params const& p) {
        float lo = 0.0002f, hi = 1.f;
        if (medianBest(p, hi) <= p.progress) {
            return hi;  // even learning a bucket at once doesn't get that far: as fast as it goes
        }
        if (medianBest(p, lo) >= p.progress) {
            return lo;
        }
        for (int i = 0; i < 14; ++i) {
            float mid = std::sqrt(lo * hi);
            (medianBest(p, mid) < p.progress ? lo : hi) = mid;
        }
        return std::sqrt(lo * hi);
    }

    inline char const* stageName(float averageSkill) {
        if (averageSkill < 0.05f) return "Noob";
        if (averageSkill < 0.20f) return "Beginner";
        if (averageSkill < 0.40f) return "Learning";
        if (averageSkill < 0.60f) return "Practiced";
        if (averageSkill < 0.80f) return "Consistent";
        if (averageSkill < 0.95f) return "Skilled";
        return "Mastered";
    }

    // the chance (0..1) that a run from `start` gets to the end now
    inline float finishChance(Params const& p, Memory const& m, float start) {
        float chance = 1.f;
        for (int b = bucketOf(start); b < BUCKETS; ++b) {
            chance *= 1.f - failChance(p, m.skill[b]);
        }
        return chance;
    }

    // "88-100% x2, 62-100%"
    inline std::string runsText(Memory const& m, size_t most = 4) {
        std::string out;
        size_t shown = 0;
        for (auto const& r : m.runs) {
            if (shown == most) {
                out += " +" + std::to_string(m.runs.size() - shown) + " more";
                break;
            }
            if (!out.empty()) {
                out += ", ";
            }
            out += std::to_string(static_cast<int>(std::lround(r.from))) + "-100%";
            if (r.count > 1) {
                out += " x" + std::to_string(r.count);
            }
            ++shown;
        }
        return out;
    }

    // saved as text: a header line, the 100 skills, then one line per finished start position run
    inline std::string serialize(Memory const& m) {
        std::ostringstream o;
        o << "cycle 1\n" << m.attempts << ' ' << m.fromStart << ' ' << m.completions << ' ' << m.best << '\n';
        for (int b = 0; b < BUCKETS; ++b) {
            o << m.skill[b] << (b + 1 < BUCKETS ? ' ' : '\n');
        }
        o << m.runs.size() << '\n';
        for (auto const& r : m.runs) {
            o << r.from << ' ' << r.count << '\n';
        }
        return o.str();
    }

    inline bool deserialize(std::string const& text, Memory& out) {
        std::istringstream in(text);
        std::string tag;
        int version = 0;
        Memory m;
        if (!(in >> tag >> version) || tag != "cycle" || version != 1) {
            return false;
        }
        if (!(in >> m.attempts >> m.fromStart >> m.completions >> m.best)) {
            return false;
        }
        for (int b = 0; b < BUCKETS; ++b) {
            if (!(in >> m.skill[b])) {
                return false;
            }
            m.skill[b] = std::clamp(m.skill[b], 0.f, 1.f);
        }
        size_t runs = 0;
        if (!(in >> runs) || runs > 1000) {
            return false;
        }
        for (size_t i = 0; i < runs; ++i) {
            Run r;
            if (!(in >> r.from >> r.count)) {
                return false;
            }
            m.runs.push_back(r);
        }
        out = m;
        return true;
    }
}

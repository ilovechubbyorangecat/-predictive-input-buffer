# The Cycle system: how it works

Cycle Mode sits on top of autoplay. Autoplay still knows how to play the level (its route, a macro, or a run from the global brain); Cycle decides **where it fails on purpose**, so the attempts look like a player learning the level. The model is in `src/cycle.hpp` (plain C++, no game code); the part that talks to the game is the "Cycle system" block in `src/main.cpp`.

## 1. The model

The level is cut into **100 buckets of 1%**. Each bucket has a **skill** from 0 (never played) to 1 (mastered). Skill belongs to the bucket and the level, not to the run.

**Chance to die in a bucket**

```
fail = choke + (noobFail - choke) * (1 - skill) ^ curve
```

- `noobFail`: chance to die in 1% it has never played (the beginner's error margin).
- `choke`: chance to die in 1% it has mastered (the error margin that never goes away).
- `curve`: the progress curve. 1 = every time through helps the same; higher = bad for longer, then quick improvement.

**Planning an attempt.** At the first step of an attempt the start percentage is read (0 from the beginning, or wherever the start position is). From that bucket on, each bucket is rolled against its fail chance. The first one that fails is where the mistake will happen, at a random point inside it. No bucket fails: the attempt has no mistake planned.

**Learning from an attempt.** When the attempt ends:

- every bucket it got through alive: `skill += gain * (1 - skill)`
- the bucket it died in: `skill += gain * deathLearn * (1 - skill)`
- `gain = rate` for a run from 0%, `rate * practiceBoost` for a run from a start position
- a start position run that reaches the end: every bucket of it becomes at least `retention`, and the run is listed as practiced ("88-100%")

Because gains shrink as skill grows, the first percents become consistent first and the frontier moves forward slowly, the way it does for a player.

**The learning rate is solved, not set.** You give a milestone: "after `attempts` attempts from 0%, a beginner's best is `progress`%". The mod simulates a few dozen beginners with that many attempts and searches for the `rate` whose median best is your milestone. It is solved again whenever a number changes (a few milliseconds).

## 2. StartPos and run memory

- Progress is the game's own percentage, read at the attempt's first step, so a start position at 88% starts the run at 88%.
- Practice from a start position trains the same buckets a run from 0% plays later. When that run reaches 88%, those buckets already have their practiced skill, so it plays them with the consistency it built there.
- A start position run to the end is kept in the list of practiced runs with how many times it was done. The readout shows them.
- One memory per level, saved in the mod's save folder under `cycle/` (a small text file: attempts, best, completions, the 100 skills, the practiced runs).

## 3. What happens in the level

1. **Attempt start**: settings are read, the level's memory is loaded, the attempt is planned.
2. **Every physics step**: the current percentage is read. Before the planned mistake autoplay plays normally.
3. **The mistake**: autoplay's inputs (route, macro, replay) are switched off. 60% of the time it is a missed click (the button stays as it was for 0.7 s), 40% a misclick (the button changes at once). If that didn't crash it, it presses at random until it dies.
4. **Death or end**: the attempt is counted and learned from, the memory is saved, the readout is updated.

Details that matter:

- A death made on purpose is **not** shown to autoplay's own learning (the route, marked spots, a macro's alignment), and that attempt is never sent to the global brain.
- Autoplay's own real deaths count as cycle attempts too.
- Restarting by hand, the game's practice mode (checkpoints) and attempts you play with Take Over are not counted.
- Turning Cycle on or changing the preset applies from the next attempt.

## 4. The presets

| Preset | Milestone | Beginner mistake / 1% | Mastered mistake / 1% | Curve | StartPos speed | Retention |
|---|---|---|---|---|---|---|
| Rookie | 10% after 60 attempts | 70% | 1.0% | 1.6 | x1.5 | 50% |
| Casual | 15% after 32 attempts | 60% | 0.6% | 1.8 | x2 | 65% |
| Grinder | 25% after 32 attempts | 50% | 0.4% | 2.0 | x2.5 | 78% |
| Prodigy | 40% after 30 attempts | 40% | 0.2% | 2.2 | x4 | 90% |
| Demon Slayer | 60% after 20 attempts | 30% | 0.08% | 2.5 | x8 | 97% |

Simulated with the model alone (not in the game), attempts from 0% until the first completion, median of 21 runs:

| Preset | Attempts to beat the level |
|---|---|
| Rookie | about 1200 |
| Casual | about 300 |
| Grinder | about 150 |
| Prodigy | about 80 |
| Demon Slayer | about 35 |

Start position practice shortens these a lot: that is what it is for.

## 5. Custom cycles

Set the preset to **Custom** (the last one after Demon Slayer, or in the settings) and edit, in the mod's settings:

| Setting | What it changes |
|---|---|
| Milestone: attempts / progress | How fast it learns ("15% after 32 attempts") |
| Beginner mistake chance | How clumsy it is in parts it has never played |
| Mastered mistake chance | How often it still chokes in parts it knows |
| Progress curve | The shape of improvement |
| Start position practice speed | How much faster a start position run teaches |
| Practice run retention | How well a finished start position run is known at once |

Tips: for a long or hard level raise the attempts or lower the progress of the milestone. For a "chokes at 9x%" feel, raise the mastered mistake chance. For practice that sticks at once, set retention to 100.

## 6. The readout and the reset

- **In the level** (top left, can be switched off): the preset, the stage (Noob, Beginner, Learning, Practiced, Consistent, Skilled, Mastered) with the average skill, the attempt number, the best from 0%, how many times it beat the level, where this run started, and the practiced runs.
- **In the window**: the same, plus the chance that a run from 0% gets to the end now.
- **Reset Cycle**: in a level it forgets that level; from the menus it forgets every level. Attempts, best, skills and practiced runs are wiped.

## 7. The utilities

- **Hide Autoplay Text** hides the AUTOPLAY label at once, Cycle or not.
- **Safe Mode** (on by default) keeps the level in the game's test mode while autoplay is on, so nothing is saved. Off, the level keeps the mode it was opened in and the game saves what autoplay does. A level opened from a start position is in test mode either way.

## 8. Where the code is

| What | Where |
|---|---|
| Model, presets, solver, save format | `src/cycle.hpp` |
| Settings, memory file, readout, mistakes | `src/main.cpp`, "the Cycle system" block (`cycleReadSettings`, `cycleLoad`, `cycleAttemptStart`, `cycleTick`, `cycleAttemptEnd`, `cycleReset`) |
| Hooks | `autoplayAttemptStart`, `autoplayTick`, `macroStep` / `macroStepXd`, `BufPlayLayer::resetLevel`, `levelComplete`, `destroyPlayer` |
| Safe Mode, Hide Autoplay Text | `applySafeMode`, `applyAutoplayText` |
| Window | `AutoplayMenu` |
| Settings | `mod.json` (`cycle-*`, `safe-mode`, `hide-autoplay-text`) |

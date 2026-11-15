# Attempt Progression & Forecasting

Autoplay can beat most levels on the first try. The progression makes it beat a level **at the attempt you choose**, and end every attempt before that at a forecast percentage.

## Turning it on
1. Open the Autoplay window (Ctrl+Q) and tick **Progression**.
2. Open a level (or its page), press **Progression** and type the attempt the level is beaten at. Press **Apply**.
3. Start autoplay. The HUD in the top left shows:

```
Attempt 3 / 5 (2 left)
This attempt: 68.4%
Next Attempt Preview: 85.5%
```

Unticking **Progression** pauses everything: no counting, no forecast, no HUD. The level's numbers stay saved.

## How the percentages are made
Each attempt's percentage comes from its number, the target and a per-level seed. Nothing is stored per attempt and nothing is generated ahead, so a target of 500,000 costs the same as 5, and jumping to attempt 311,402 is instant.

- **Reach**: a curve from 0% to 100% at the target. It is the furthest an attempt can get.
- **Outcome**: a point at or below the reach. With a short target every attempt is close to it. With a long one most attempts die early and a few get to the reach.

| Target | What it looks like |
|---|---|
| 5 | 14.7%, 41.5%, 68.4%, 85.5%, 100% |
| 40 | 7, 12, 0.4, 19, 21, 22, 26, 32, 30, 13, 39 ... 91, 94, 82, 98, 100 |
| 500,000 | under 3% for the first thousands; around attempt 250,000 the best is 55% and the average 16%; attempt 500,000 is 100% |

The attempt at the target is always 100%. After the level is beaten, about 6 attempts in 10 beat it again.

## Checkpoints
In the Progression window, type `attempt:percent` pairs: `200:60, 500:87, 762:97`. Those attempts end at exactly those percentages. The reach curve goes through them, so the attempts between are filled in to match. Clear the box to remove them. A checkpoint at or past the target is ignored.

## Start positions
A run from a start position has its own attempt count. Its target is its share of the level at half the pace: with a target of 500, runs from 88% are beaten at attempt 30.

When such a run reaches 100%, that start is **practiced**. From then on an attempt from 0% (or an earlier start) that gets to 88% clears the rest 9 times in 10. This can beat the level before the target. Checkpoints apply to runs from 0% only.

## How an attempt ends (setting)
- **Natural** (default): a quarter of a second before the percentage, autoplay misses a click or misclicks, and crashes. The death lands near the forecast, not exactly on it; in a part where a wrong input doesn't kill quickly it can be later.
- **Exact**: it plays perfectly and the player dies right at the percentage, even in mid-air.

If autoplay dies on its own before the forecast, the attempt counts with the percentage it really got.

Not counted: attempts in practice mode, an attempt you restart by hand, an attempt you play in with Take Over, and the attempt autoplay was switched on in the middle of.

## The Progression window
| Field | What it does |
|---|---|
| Beat the level at attempt | The target. Changing it re-paces every attempt from the next one. |
| Next attempt number (jump) | The number the next attempt from 0% gets. |
| Level total attempts (saved) | Overwrites the total attempts the game has saved for the level. The progression then counts on from that number. Works with Progression off. |
| Checkpoints | See above. |
| Reset | Back to the default target, no checkpoints, no practiced runs. |

An attempt already in progress keeps its percentage; edits apply from the next one.

## Hide Capture
With it on, the AUTOPLAY label and the forecast HUD are not drawn in the game, so no recording has them.

- **Windows 10 (2004) or newer**: they are drawn in a see-through window over the game that screen, window and game capture leave out. You see them, OBS doesn't. The window shows only while the game is in front.
- **Older Windows, Android, macOS, iOS**: the texts are only hidden.
- The Autoplay window (Ctrl+Q) and the console (F9) are part of the game's picture. They are recorded while open.

## Where it is saved
`<mod save folder>/progression/level_<id>.txt`, one file per level.

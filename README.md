# Autoplay (Geode)

By Indonesiangamer777.

Autoplay for level showcases. That is all this mod does now: the input buffer (delayed clicks, stages, auto-release, level memory, path view, training hints, stats, heatmap) has been removed.

With Safe Mode on (the default) the level is put in test mode and nothing autoplay plays is saved as progress. The AUTOPLAY label is on screen unless you hide it. Don't pass its runs off as your own: with Safe Mode off the game saves them to your account, and submitting a botted run to a leaderboard or a list breaks their rules.

## Keys
Ctrl+Q window | F8 copy the console log | F9 open/close the in-game console

Ctrl+Q opens the window anywhere (in a level it pauses first), and Ctrl+Q again closes it. There is no button for it in the pause menu or on the main menu, so on a phone without a keyboard the window can't be opened. The keys can be changed in the mod's settings or in Geode's keybinds popup.

## Autoplay
- **Route planner**: plans a route up to 10 s ahead while it plays, with a pathfinder for wave and ship, and it learns from its deaths. What it learned on a level is saved (Forget Level clears it).
- **Macros**: plays GD Replay (GDR) and xdBot macros: your own in the Macros Folder, or one it downloads from [Hyperbolus](https://hyperbolus.net) (used with its owner's permission, every macro credited), MacroDash or Astral. Choose Macro lets you pick one yourself. It lines the macro up by itself, keeps you on its path with the bot's position records, plans the parts a macro dies in, and plays 2-player levels with each player's clicks.
- **Global brain**: a run autoplay finishes is shared, and every game's autoplay can play the runs others finished. A level the brain has a run of always plays it.
- **Take Over**: your own clicks come first, and autoplay plays on when you stop.
- Platformer levels aren't supported.

## Cycle: autoplay that learns like a player
Cycle Mode makes autoplay play like someone learning the level. It starts as a complete beginner, makes mistakes and dies, and gets further and more consistent with every attempt.

- **Presets**: Rookie, Casual, Grinder, Prodigy, Demon Slayer, from slowest to fastest learner. Custom lets you set the milestone ("15% after 32 attempts"), the beginner and mastered mistake chances, the progress curve, and how much start position practice counts.
- **Start positions**: progress is measured from where the run starts. Skill is kept per 1% of the level, so a part practiced from a start position is known when a run from 0% reaches it. A start position run that reaches the end is listed as practiced (for example "88-100% x2").
- **Readout**: the top left shows the preset, the stage and skill, the attempt, the best, and the practiced runs. The window shows the same and the chance to beat the level from 0% now.
- **Reset Cycle**: forgets the level you are in (or every level, from the menus): it is a beginner again.
- Autoplay has to be able to play the level for this to work: Cycle only adds the mistakes. Autoplay's own deaths count as attempts too.

How it works and how to tune it: [docs/CYCLE.md](docs/CYCLE.md).

## Bot utilities (with or without Cycle)
- **Hide Autoplay Text**: hides the AUTOPLAY label.
- **Safe Mode** (on by default): nothing autoplay plays is saved to your account. Off: the game saves completions, percentages and stats as if you had played.

## The window
One popup: Start / Stop Autoplay; the Take Over, Cycle Mode, Safe Mode and Hide Autoplay Text checkboxes; the cycle preset and what the cycle has learned on this level; Choose Macro, Macros Folder, Forget Level, Forget Macro, Reset Cycle, and Settings (which opens the mod's Geode settings). The log is on the in-game console (F9), and F8 copies it.

## Online (each can be turned off in the settings)
- An anonymous id with your device type and version every 90 s (the online counter).
- Autoplay's finished runs (the level and the button presses), for the global brain.
- Macro downloads.

## Settings
- Autoplay Label (big at the top, or small in the corner for showcases) and Show Autoplay Trail.
- Hide Autoplay Text, Safe Mode.
- Cycle Mode, the cycle preset, Show cycle progress, and the Custom cycle's numbers.
- Global Brain, Online Macros, Macro Position Fixes, Macro Plays At Its TPS, Play xdBot Macros Like xdBot, Macro Timing Offset.
- Physics TPS and the planning time budget.
- Ship click limit (CPS): the most clicks per second autoplay's own route uses in ship (10 by default). A part it gets stuck in is tried again with faster clicks. Wave isn't limited, and a macro plays its own clicks.
- Prediction for autoplay's hidden player: move triggers, moving objects, through portals.
- Debug: route step check, log trigger checks, a Windows console window.

## Building
A normal Geode mod: `geode build`, or the Build Mod workflow (Windows, Android 32/64, macOS, iOS). The site and the global brain's server are in `web/`.

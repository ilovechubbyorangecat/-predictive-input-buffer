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

## Attempt Progression & Forecasting
You choose **at which attempt a level is beaten**; every attempt before it ends at a forecast percentage. Autoplay plays normally up to that percentage and dies there, so a showcase looks like a player getting further over time. Full guide: [docs/PROGRESSION.md](docs/PROGRESSION.md).

- **Progression** checkbox: on / off at any time. Off: nothing is counted or forecast, no HUD, autoplay plays as well as it can. The level's numbers are kept.
- **The target scales everything.** A target of 5 gives something like 15%, 42%, 68%, 86%, 100%. A target of 500,000 dies in the first few percent for thousands of attempts and creeps up. Nothing is generated ahead: each attempt's percentage is worked out from its number, so any target (up to 100,000,000) and any jump cost the same.
- **Ups and downs**: attempts fall below the best so far, more often the longer the target.
- **Checkpoints**: lock a percentage to an attempt (`200:60, 500:87, 762:97`); the attempts between them are filled in.
- **Start positions**: a run from a start position has its own count and a shorter target. Once it reaches 100% it is "practiced": a later run from 0% (or an earlier start) that gets that far clears the rest 9 times in 10.
- **HUD** (top left): the attempt and how many are left, this attempt's percentage and `Next Attempt Preview: 12%`. **Forecast HUD** hides only the text.
- **Progression** button: the target, the next attempt's number (jump), the checkpoints and the level's saved total attempts, with the next attempts listed.

## Utilities (with or without the progression)
- **Safe Mode** (on by default): while autoplay is on the level is in the game's test mode, so no completion, progress or stats are saved. Turning it off lets bot runs save to your account; submitting those breaks leaderboard and list rules.
- **Hide Autoplay Text**: hides the AUTOPLAY label.
- **Hide Capture**: takes the mod's texts in the level (AUTOPLAY, the forecast HUD) out of the game's picture, so recordings don't have them. On Windows 10 (2004) or newer you still see them, in a see-through window over the game that capture leaves out. On Android, macOS and iOS they are only hidden. The Autoplay window and the console are recorded while open.
- **Level total attempts** (in the Progression window): overwrites the total attempts the game has saved for the level (Clubstep 20 to 60); the progression counts on from the new number.

## The window
One popup: Start / Stop Autoplay; six checkboxes (Take Over, Progression, Safe Mode, Forecast HUD, Hide Autoplay Text, Hide Capture); this level's progression; and the buttons Choose Macro, Macros Folder, Forget Level, Forget Macro, Progression and Settings.

## Online (each can be turned off in the settings)
- An anonymous id with your device type and version every 90 s (the online counter).
- Autoplay's finished runs (the level and the button presses), for the global brain.
- Macro downloads.

## Settings
- Autoplay Label (big at the top, or small in the corner for showcases) and Show Autoplay Trail.
- Hide Autoplay Text, Safe Mode.
- Attempt Progression, Show forecast HUD, How an attempt ends (Natural / Exact), the default target, and Hide Capture.
- Global Brain, Online Macros, Macro Position Fixes, Macro Plays At Its TPS, Play xdBot Macros Like xdBot, Macro Timing Offset.
- Physics TPS and the planning time budget.
- Ship click limit (CPS): the most clicks per second autoplay's own route uses in ship (10 by default). A part it gets stuck in is tried again with faster clicks. Wave isn't limited, and a macro plays its own clicks.
- Prediction for autoplay's hidden player: move triggers, moving objects, through portals.
- Debug: route step check, log trigger checks, a Windows console window.

## Building
A normal Geode mod: `geode build`, or the Build Mod workflow (Windows, Android 32/64, macOS, iOS). The site and the global brain's server are in `web/`.

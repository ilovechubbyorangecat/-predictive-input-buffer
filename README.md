# Click Before Frames (Geode)

Autoplay for level showcases. That is all this mod does now: the input buffer (delayed clicks, stages, auto-release, level memory, path view, training hints, stats, heatmap) has been removed.

Not for verified or leaderboard runs: the level is put in test mode, nothing autoplay plays is saved as progress, and AUTOPLAY stays on screen the whole time.

## Keys
7 menu | F8 copy the console log | F9 open/close the in-game console

The keys can be changed in the mod's settings or in Geode's keybinds popup. The menu also opens from the button on the main menu and in the pause menu.

## Autoplay
- **Route planner**: plans a route up to 10 s ahead while it plays, with a pathfinder for wave and ship, and it learns from its deaths. What it learned on a level is saved (Forget Level clears it).
- **Macros**: plays GD Replay (GDR) and xdBot macros: your own in the Macros Folder, or one it downloads from [Hyperbolus](https://hyperbolus.net) (used with its owner's permission, every macro credited), MacroDash or Astral. Choose Macro lets you pick one yourself. It lines the macro up by itself, keeps you on its path with the bot's position records, plans the parts a macro dies in, and plays 2-player levels with each player's clicks.
- **Global brain**: a run autoplay finishes is shared, and every game's autoplay can play the runs others finished. A level the brain has a run of always plays it.
- **Take Over**: your own clicks come first, and autoplay plays on when you stop.
- Platformer levels aren't supported.

## The window
- **Log**: what autoplay did, with Copy Log and the on-screen console.
- **Autoplay**: start / stop, Take Over, Forget Level, and the macro and global brain switches (Choose Macro, Macros Folder, Forget Macro).
- **Account**: link the game to your Discord on the Click Before Frames site, and your autoplay's stats. Autoplay and updates need the game linked.
- **Settings** (the gear): window size and opacity, the pause menu button, Physics TPS, the prediction switches and updates.

## Online (each can be turned off in the settings)
- An anonymous id with your device type and version every 90 s (the online counter).
- Autoplay's finished runs (the level and the button presses), for the global brain.
- Linking the game to your Discord on the site.
- Macro downloads.

## Settings
- Autoplay Label (big at the top, or small in the corner for showcases) and Show Autoplay Trail.
- Global Brain, Online Macros, Macro Position Fixes, Macro Plays At Its TPS, Play xdBot Macros Like xdBot, Macro Timing Offset.
- Physics TPS and the planning time budget.
- Prediction for autoplay's hidden player: move triggers, moving objects, through portals.
- Debug: route step check, log trigger checks, a Windows console window.

## Building
A normal Geode mod: `geode build`, or the Build Mod workflow (Windows, Android 32/64, macOS, iOS). The site and the global brain's server are in `web/`.

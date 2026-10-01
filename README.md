> [!WARNING]
> **This repository is archived and no longer maintained.**
> The slow animation fix now lives in **[GameFixes_mm](https://github.com/SlynxCZ/GameFixes_mm)**, one Metamod:Source plugin that bundles all of my CS2 server fixes. Each fix is turned on or off on its own in the config.
> Release binaries here were built against an older Metamod and may not load on current Metamod builds (plugin API 18).

## Migrating to GameFixes_mm

1. Remove `addons/slow_animation_fix/` and `addons/metamod/slow_animation_fix.vdf` from your server.
2. Install the latest **[GameFixes_mm release](https://github.com/SlynxCZ/GameFixes_mm/releases)**.
3. Enable the `slow_animation` block in `addons/game_fixes/game_fixes.ini`:

   ```
   "slow_animation"
   {
   	"enable"	"1"
   }
   ```

Issues and PRs go to [GameFixes_mm](https://github.com/SlynxCZ/GameFixes_mm).

---

# SlowAnimationFix_mm

**Metamod plugin fixing slow-motion/sluggish animation bug on CS2 servers.**

---

## Problem

After a CS2 server has been running the same map for a long time, `curtime` in `CGlobalVars` accumulates as a 32-bit float. As it grows, floating-point precision degrades — at ~24 h uptime the precision drops to half a tick, at ~48 h it exceeds a full tick interval. This causes animation, movement, and lag-compensation to produce incorrect results.

Valve never added a reset mechanism for `curtime` in any code path.

---

## What it does

On every map start (`StartupServer`) the plugin snapshots the current map name. Then, every **30 minutes**, a repeating timer checks the server:

* Player slots 0–63 are scanned for **real players** (bots and GOTV are ignored via the `FL_FAKECLIENT` flag).
* If **at least one human** is connected, nothing happens — the check simply runs again 30 minutes later.
* If the server is **empty**, a changelevel back to the same map is performed, resetting `curtime`:

  * **Regular map** — calls `IVEngineServer2::ChangeLevel` directly.
  * **Workshop map** — issues a `ds_workshop_changelevel <map>` server command (detected via `IsMapValid`).

The timer survives map changes, so the cycle continues indefinitely.

---

## Result

* Server can run indefinitely on the same map without the slow-motion bug appearing
* No manual restarts required
* Zero impact during active gameplay — the reload only ever happens on an empty server
* Fully self-contained — no external Utils/Players plugins required

---

## Notes

* Uses a self-contained scheduler (`scheduler.h` / `scheduler.cpp`) ticked from `ISource2Server::GameFrame`, modelled after [Source2Toolkit](https://github.com/SlynxCZ).
* Player detection is done directly through the entity system (`CBasePlayerController` + schema fields), no external player-manager plugin needed.
* Tested on 64-tick dedicated servers.

---

## Requirements

* Metamod:Source (CS2)

---

## Author

Slynx (˙·٠● S l y n x ●٠·˙)  
[https://slynxdev.cz](https://slynxdev.cz)

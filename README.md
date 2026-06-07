# SlowAnimationFix_mm

**Metamod plugin fixing slow-motion/sluggish animation bug on CS2 servers.**

---

## Problem

After a CS2 server has been running the same map for a long time, `curtime` in `CGlobalVars` accumulates as a 32-bit float. As it grows, floating-point precision degrades — at ~24 h uptime the precision drops to half a tick, at ~48 h it exceeds a full tick interval. This causes animation, movement, and lag-compensation to produce incorrect results.

Valve never added a reset mechanism for `curtime` in any code path.

---

## What it does

Every **30 minutes** the plugin checks whether the server is empty (no connected players) **and** whether `curtime` has exceeded the 1-hour threshold where precision issues begin.

When both conditions are met the plugin performs a changelevel back to the same map:

* **Regular map** — calls `IVEngineServer2::ChangeLevel` directly.
* **Workshop map** — issues a `ds_workshop_changelevel <map>` server command (detected via `IsMapValid`).

### mp_timelimit compensation

To avoid players losing round time after an invisible fix-triggered map reload, the plugin automatically adjusts `mp_timelimit` on the new map:

```
new_mp_timelimit = original_mp_timelimit - elapsed_minutes_before_reload
```

For example, if a 60-minute map was 45 minutes in when the fix fired, the reloaded map starts with `mp_timelimit 15` so the total round duration stays 60 minutes.

---

## Result

* Server can run indefinitely on the same map without the slow-motion bug appearing
* No manual restarts required
* Zero impact during active gameplay — the check only acts on an empty server
* Round time is seamlessly preserved via automatic `mp_timelimit` correction

---

## Notes

* The check runs every 30 minutes and only triggers after `curtime` exceeds **1 hour** (`CURTIME_THRESHOLD`).  Both constants are `constexpr` at the top of `plugin.cpp`.
* Uses a self-contained scheduler (`scheduler.h` / `scheduler.cpp`) ticked from `ISource2Server::GameFrame`, modelled after [Source2Toolkit](https://github.com/SlynxCZ).
* Tested on 64-tick dedicated servers.

---

## Requirements

* Metamod:Source (CS2)

---

## Author

Slynx (˙·٠● S l y n x ●٠·˙)  
[https://slynxdev.cz](https://slynxdev.cz)

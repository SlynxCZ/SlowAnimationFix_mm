# SlowAnimationFix_mm

**Experimental metamod plugin fixing slow-motion/sluggish animation bug on CS2 servers.**

---

## Problem

After a CS2 server has been running the same map for a long time without a map change or restart, players experience slow-motion animations and sluggish movement when joining.

**Root cause:** `curtime` in `CGlobalVars` is a 32-bit float. As it grows larger over time, floating point precision degrades — at ~24h uptime the precision drops to half a tick, at ~48h it exceeds a full tick interval. This causes animation, movement and lag compensation to produce incorrect results.

Valve never added a reset mechanism for `curtime` in any code path.

---

## What it does

* Hooks `ISource2Server::GameFrame` every tick
* When `curtime` exceeds a threshold **and** the server is empty, subtracts a large offset from `curtime` and adjusts `tickcount` accordingly
* Preserves all relative time differences — entity timers, think callbacks and scheduled events are unaffected

---

## Result

* Server can run indefinitely on the same map without the slow-motion bug appearing
* No map restarts required
* Zero impact during active gameplay — correction only happens on an empty server

---

## Notes

* Marked **experimental** — the fix is based on reversed `CGlobalVars` offsets from `libserver.so` and `libengine2.so`
* Tested on 64-tick dedicated servers
* Default threshold: **1 hour** of accumulated curtime on an empty server

---

## Requirements

* Metamod:Source (CS2)

---

## Author

Slynx (˙·٠● S l y n x ●٠·˙)
[https://slynxdev.cz](https://slynxdev.cz)
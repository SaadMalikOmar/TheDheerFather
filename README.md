# Untitled Horror Game — Design Doc

An asymmetric multiplayer horror game: **1 Killer vs N Runners**, multiple game modes,
buyable characters, and an item shop. Meme-horror tone built around a friend group.

> Lineage: Dead by Daylight / Identity V structure, but with a comedic cast and a
> signature **PUD (cowardice) system** that drives the horror.

## Index

| File | Contents |
|------|----------|
| [design-pillars.md](design-pillars.md) | Core systems: PUD, Kealth, stillness, scare tactics, open questions |
| [weapons.md](weapons.md) | Runner weapons + the damage-vs-killer question |
| [killers.md](killers.md) | Skinny Bear, Lucki, Tung Tung (full kit) |
| [runners.md](runners.md) | The 6 base characters (Scout, Tank, Engineer, Trickster, Stealth, Medic) |
| [gamemodes.md](gamemodes.md) | Game Mode 1: Load Shedding |
| [difficulties.md](difficulties.md) | Casual, Normal, Hardcore — selection rules, killer lethality, Hardcore ability limits |
| [tech-stack.md](tech-stack.md) | Engine (UE5), online stack (EOS), platforms, PS5 reality check |

## Design pillars (one-liners)

1. **Asymmetry through info, not just stats** — killers win by *finding* you; runners win by *hiding* and completing objectives together.
2. **PUD = personality** — every runner reacts to fear differently. Fear is a gameplay stat, not just a screen filter.
3. **Each killer is a tempo/playstyle**, not a stat block — Skinny Bear (ambush/stillness), Lucki (vehicle/attrition), Tung Tung (FPS skill/kidnap).
4. **Comedy-horror tone** — the cast is the joke; the horror is real.

## Normalized speed scale (IMPORTANT)

All speeds below are on **one scale where `100` = average human jog**. Runner baseline
(Scout/Dheer) is `60`. This replaces the mixed scales in the original notes. Raw original
numbers are preserved in each character file; the normalized value is what the game should use.

| Character | Role | Speed (norm) | Notes |
|-----------|------|-------------|-------|
| Skinny Bear | Killer | 250 | **Intentionally inescapable on foot** — counter is stillness |
| Shaun (Stealth) | Runner | 100 | Fastest runner |
| Julius (Engineer) | Runner | 70 | |
| Mahnam (Medic) | Runner | 70 | |
| Dheer (Scout) | Runner | 60 | Baseline |
| Musa (Trickster) | Runner | 50 | |
| Lucki (on foot, normal) | Killer | 50 | Slower than most runners — relies on car |
| Troos (Tank) | Runner | 40 | |
| Lucki (hungry) | Killer | 5 | Crawls until he eats |
| Lucki (Diet Coke) | Killer | 150 | Temporary boost |
| Lucki (Prius) | Killer | — | Define separately; one-shot ram |
| Tung Tung | Killer | **~120** | Slows per victim carried; FPS-style movement |

> **Open balance note:** Skinny Bear at 250 means no runner ever escapes a straight chase.
> That's intended *only if* runners have a reliable, fast "go still" action and the 3s kill
> animation gives a real rescue window. See [design-pillars.md](design-pillars.md).

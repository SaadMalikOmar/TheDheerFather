<!-- Copyright (c) 2026 Saad Malik Omar. All Rights Reserved. -->
# The Dheer Father

An asymmetric multiplayer horror game — **1 killer vs N runners** — built in **Unreal Engine 5.8**.
Dead by Daylight / Identity V structure, comedic cast, and a signature **PUD (cowardice) system**
that makes fear an actual gameplay stat.

> Status: **playable v0.4**, online over Steam. Greybox art; all systems working.

---

## Play it

**Requirements:** Windows + **Steam running** (any account, logged in — no purchase, no setup).

1. Extract the build, run `Windows/TheDheerFather.exe`.
2. Everyone lands on the **Party Lobby** island.
3. Host presses **F1**. Everyone else presses **F2** — works over the internet, no port forwarding.
4. Party leader picks settings: **M** map · **G** generators · **K** killer on/off · **8/9/0** difficulty.
5. Leader presses **H** to launch → pick characters with **B** + number → **H** again to start.

Full control list: **[controls.md](controls.md)**

---

## Game Mode 1 — Load Shedding

You and your mates are in your own houses, on your setups, in a Discord call. Then the internet dies.

| Stage | What happens |
|-------|--------------|
| **1a — Check the router** | Spawn at your PC. *"CONNECTION LOST."* Go downstairs, **E** on your router. |
| **1b — Fix the fibre** | Out into the street; repair the fibre boxes scattered across the road and forest. |
| **1c — Get home** | Internet's back — everyone runs home and sits at their PC. When the last player sits… |
| **⚡ BLACKOUT** | The power dies. The killer is unleashed. |
| **2a — Generators** | Grab the **repair kit** from your house. Two players per generator: one repairs, one on lookout. |
| **2b — The Roadworks** | Everyone still alive gathers at the closed end of the hill and flips the power back on **together**. |

---

## The cast

**Runners** — Dheer (Scout, smart watch), Troos (Tank, can't climb), Julius (Engineer, 3× repairs),
Musa (Trickster, decoys + hidden voice-note speakers), Shaun (Stealth, invisible when still, dies in
one hit), Emma (Medic, rave stash heals + kills fear).

**Killers** — **Skinny Bear** (third person, one-shot, blind to still prey, sniffs through walls),
**Lucki** (the blue Prius: keys, ignition, hybrid fuel, headlights — plus DJ the dog with scout/attack/
defend AI), **Tung Tung** (disguises as a tree with one red leaf, drags victims to his shrine).

Details: [runners.md](runners.md) · [killers.md](killers.md) · [weapons.md](weapons.md) ·
[gamemodes.md](gamemodes.md) · [difficulties.md](difficulties.md) · [design-pillars.md](design-pillars.md)

---

## Maps

Two are greyboxed from **real OpenStreetMap data**:

- **Whyteleafe Hill** — the real road, ~110 semi-detached houses, green belt on the other side.
- **WonderPond** — the Waddon district, Croydon: real streets, buildings, ponds, and the Esso
  (in-game: *MOST HATED S.O*) exactly where it stands in real life.
- **Whitewood** — original dense procedural forest.
- **PartyLobby** — the pre-game hangout island.

House layouts, loot, and objectives re-roll every round from a server seed all clients share.

---

## Build from source

**Requirements:** UE 5.8, Visual Studio 2022+ with the C++ workload (MSVC ≥ 14.38).

```bash
git clone <this repo>
```

Then either open `TheDheerFather.uproject`, or build from the command line:

```
"<UE5.8>/Engine/Build/BatchFiles/Build.bat" TheDheerFatherEditor Win64 Development -Project="<path>/TheDheerFather.uproject" -WaitMutex -FromMsBuild
```

Package a shippable build:

```
"<UE5.8>/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project="<path>/TheDheerFather.uproject" -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="<out>"
```

> Use **Development**, not Shipping — the HUD currently draws through on-screen debug messages.

All gameplay is C++ (`Source/TheDheerFather`, `Tdf` class prefix) using GAS for abilities.

---

## Roadmap

- [ ] Character models + animations (rigged skeletons, retargeted anims)
- [ ] Proximity voice chat (needs EOS — the Discord-call-in-game from the design doc)
- [ ] Real audio pass (Musa's voice-note playback needs mic capture)
- [ ] Runner-driveable Prius
- [ ] BearCampus map (currently a bare landscape)
- [ ] Ship networking: own Steam App ID or EOS (currently Valve's shared test AppID 480)

---

## Licence

**Copyright © 2026 Saad Malik Omar. All Rights Reserved.**

This project is **proprietary**. The code, design, characters, and assets may not be
used, copied, modified, or distributed without express written permission.
See **[LICENSE](LICENSE)** for the full terms.

Unreal Engine © Epic Games, Inc.

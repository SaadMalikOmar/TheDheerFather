# Game Modes

The game supports **multiple game mode types**. Mode 1 is defined below; reserve this file for
additional modes as they're designed.

---

## Game Mode 1 — LOAD SHEDDING

### Map

- An **uphill road, ~half a mile long.**
- **One side:** semi-detached houses running up the road.
- **Other side:** a **green-belt forest.**
- Both **top and bottom** of the road are **closed for construction** (the map boundaries).
- **110 houses.** Every character's spawn is inside one of the houses; players are scattered
  between them. The map is large → finding players takes time.

### Map Story (cold open)

All players start in their **own houses, in their rooms, at their setups, in a Discord call** —
so **proximity chat** works when they're near their monitor. The POV opens on each player as their
character playing an **FPS**. Suddenly everyone **loses connection** (internet cuts out) but voice
chat still works: *"wth, our internet cut out"* → *"let's go check it out."*

### Stage 1 — Back Online

| # | Task |
|---|------|
| 1 | **Check router.** |
| 2 | **Repair all fibre-optic boxes** — broken boxes scattered **randomly across the street / in the forest**, *not* near houses. |
| 3 | **Everyone go home, lock doors, rejoin the game.** When everyone is reconnected to their computer… |

→ **All electricity dies.**

### Stage 2 — Load Shedding

| # | Task |
|---|------|
| 1 | Find a **repair kit** in your home, then **locate a power generator** (ratio **1 generator : 2 players** → **3 generators** spawned at random houses/forest). **2 players per generator:** one **repairs**, one **stands lookout with a weapon** to buy time if found. |
| 2 | All survivors **meet at the roadworks** — **50/50** chance it's at the **top or bottom** of the hill. When everyone alive has gathered, **turn the electricity back on** → safe. Wait there for all players to **win**. |

### ✅ IMPLEMENTED (current build) — how it actually works in-game

**House types (3), shuffled every round by a server seed all clients share:**
1. **Spawn houses (7)** — two storeys, stairs, furniture, bedroom PC setup, router, wardrobe
   (hide spot), **repair kit** on the ground floor. Each player claims their OWN house for the match.
2. **Loot houses (~30% of the rest)** — front door open: **45%** food (heal +25 / Lucki +40 hunger),
   **30%** a weapon (rarity by strength: Taser 50% / Axon 30% / Triple T 20%).
3. **Locked houses (~70%)** — solid front wall, no entry. Street = semi-detached PAIRS (~110 doors).

**Phase chain:** Lobby → **CheckRouter** (E on your router, beacons mark them) →
**FixInternet** (fibre boxes, scattered away from houses) → **ReturnHome** ("internet's back!" —
everyone sits at a PC; free monitor seats get beacons) → **BLACKOUT** → **Repair** (generators)
→ **Escape** (roadworks, 50/50 top/bottom). Match opens with every runner auto-seated at their PC:
*"!! CONNECTION LOST !! …voice chat still up."*

- **Killer entry:** the killer is in the world from the start but **cannot attack until the
  blackout** (`KillersUnleashed` = Repair/Escape phases only). DJ can roam/track earlier, bites
  only after dark.
- **Generators:** need a **repair kit** (from your house) + a **2nd runner in lookout range**, or
  progress stops. Exception: last runner standing works solo. Julius 3×, Dheer ⅓×.
- **Maps without starter houses** auto-skip router/monitor stages (party lobby island).
- **Win/lose:** runners win when everyone alive reaches the roadworks; killer wins when nobody's
  left free (dead, or tied to Tung Tung's shrine).
- **Still to come:** proximity voice near monitors (needs EOS Voice → Epic account), generator
  count scaling by lobby size (party leader can set 1–6 manually with **G**).

---

## Future modes (placeholder)

> You mentioned *multiple* game mode types. Drop ideas here as headers and we'll flesh them out.
- [ ] Mode 2 — TBD
- [ ] Mode 3 — TBD

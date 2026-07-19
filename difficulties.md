# Difficulty Modes

Three modes that change **character selection, killer lethality, and ability limits.** All modes
use the same maps and game mode objectives — only the meta-rules change.

---

## Casual

**Vibe:** Friends messing around, learners, low-stakes. Still tense, but forgiving and more chaotic.

### Selection
- **Runners** choose their character manually in the lobby — **one of each** per game (no duplicates).
  First-come-first-served or a pick-order system.
- **Killer** manually selects which killer they want to play.
- Runners can also choose which weapons they bring in (from unlocked inventory).

### Rules
- **Weapons are super scarce** — fewer spawns across the map; players have to fight over them or
  go searching. Creates resource tension even among teammates.
- **The killer CAN be killed.** Runner weapons deal full damage to the killer's Kealth. If Kealth
  hits 0, the killer is killed and runners win immediately.
- Standard ability recharge rules apply (no limits).

---

## Normal

**Vibe:** The balanced, intended experience. Some chaos, some skill.

### Selection
- Characters are **randomly allocated** — one runner per character, no choice. Killer is **random** too.
- Players do not know who they'll get until the game starts.

### Rules
- **The killer can be knocked** — weapons can deplete Kealth and temporarily knock the killer out
  (stun/down state), buying runners time to escape or complete objectives. Killer gets back up.
- Killer is **not killable** — only stun-able. Runner weapons still matter for survival.
- Standard ability recharge rules apply.

---

## Hardcore

**Vibe:** Full horror. No safety net. Everything is scarcer, messier, and more dangerous.

### Selection
- Characters are **randomly allocated** — one runner per character, no choice. Killer is **random** too.

### Rules

#### Killer invincibility
- The killer **cannot be killed or knocked out.** Runner weapons only **distort** the killer
  (brief visual/audio interference, slight slowdown) — they do not down or stagger.
- This means weapons serve purely as **delay tools**, not win conditions.

#### Friendly fire — tripping
- **All runners can trip each other** while running away from the killer.
  - Tripping a teammate causes them to stumble and slow briefly — in a panic, runners running in
    the same direction can knock into each other.
  - Recommend: proximity-based, triggers when two runners collide at sprint speed. Adds chaos and
    blame moments.

#### Ability limits — no recharge
Killer abilities do **not recharge** in Hardcore. Each killer has a fixed pool of uses:

**Skinny Bear**
| Ability | Hardcore limit |
|---------|:----------:|
| Tactical — Mimic player voice | **7 uses** |
| Ultimate — Scream echolocation | **1 use** |

**Lucki**
| Ability | Hardcore limit |
|---------|:----------:|
| Tactical — Spirit of Nani (Ring doorbell) | **4 Nanis** |
| Tactical — Anju door trap | **5 Anjus** |
| Ultimate — DJ | **1 DJ** — if killed, **does not respawn** |
| Tactical — Go Invisible | *(define use limit — suggest 3 uses)* |

**Tung Tung**
| Ability | Hardcore limit |
|---------|:----------:|
| Tactical — Ground smash | *(suggest 5 uses — can no longer recharge every 30s)* |
| Ultimate — Root of Flesh | *(suggest 2 uses)* |

> Tung Tung Hardcore limits are **TBD** — fill in once his kit is fully playtested. The pattern
> (scarce ults, moderate tacticals) is consistent with the other killers.

#### Hardcore summary vs other modes

| Rule | Casual | Normal | Hardcore |
|------|:------:|:------:|:--------:|
| Character selection | Manual pick | Random | Random |
| Killer selection | Manual pick | Random | Random |
| Weapon scarcity | Very scarce | Normal | Normal |
| Killer killable? | ✅ Yes | ❌ (stun only) | ❌ (distort only) |
| Ability limits | Unlimited recharge | Unlimited recharge | Hard cap, no recharge |
| Runners trip each other | ❌ | ❌ | ✅ |

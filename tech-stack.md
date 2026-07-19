# Tech Stack & Platform Decisions

> Status: **DECIDED** (engine + online stack). Console release gated on Sony partnership.

## Target platforms
- **PC** (primary / first to build)
- **PS5** (cross-play with PC)
- Designed cross-platform from day one (controller-first input, scalable UI, EOS online).

## Engine — Unreal Engine 5 ✅

Chosen over Unity for this specific game. Reasons:
1. **Dead by Daylight (the reference game) is built on Unreal** — proven path for asymmetric horror MP.
2. **First-class PS5 support** — mature console toolchain.
3. **Horror visuals**: Lumen (dynamic lighting), Nanite (dense forest + 110 houses), built-in
   post-processing for PUD distortion/fog effects.
4. **MetaHuman** — covers most runner models for free (killers still need custom modeling).
5. **Gameplay Ability System (GAS)** — purpose-built for abilities with cooldowns, charges, and
   limited uses. Directly maps to our design: Hardcore ability caps (7 mimics / 1 ult, 4 Nanis,
   etc.) vs. cooldown-based recharge in Normal/Casual. See [difficulties.md](difficulties.md).

## Online services — Epic Online Services (EOS) ✅

Free, cross-platform (PC ↔ PS5), pairs natively with UE5. Covers the entire multiplayer social
layer requested:

| Need | EOS feature |
|------|-------------|
| Queue for games (DbD-style matchmaking) | Sessions / Matchmaking |
| Lobby — friends join before queue | Lobbies |
| Play with friends | Friends + Parties |
| Custom games | Lobbies (private) |
| Proximity voice chat | EOS Voice |
| Cross-play PC ↔ PS5 | Cross-platform by design |

## Architecture choices

| Layer | Choice | Why |
|-------|--------|-----|
| Engine | Unreal Engine 5 | See above |
| Code | C++ (systems) + Blueprints (iteration) | Standard UE workflow |
| Abilities | Gameplay Ability System (GAS) | Killer abilities, cooldowns, Hardcore caps |
| Online | Epic Online Services (EOS) | Matchmaking, lobby, party, voice, cross-play |
| Servers | **Dedicated servers** | Anti-cheat + fairness. P2P invites cheating in a |
|         |                        | killer-vs-runners game where one player has god-mode info. |

## ⚠️ PS5 reality check (business/legal gate — not technical)

Shipping to PS5 requires this regardless of engine:
- **PlayStation Partner Program** — must be an accepted, registered developer. Application +
  approval process; you receive official dev kits.
- **Cross-play & store release require Sony certification** (TRC compliance).
- **Plan:** build & ship the **PC version first**, designed cross-platform. Pursue the Sony
  partnership in parallel. UE5's PS5 toolchain makes the eventual port the easy part.

## Build order implication
1. PC vertical slice first (one map, one killer, core loop).
2. EOS lobby/party/matchmaking early — it shapes everything.
3. Console port once Sony partnership is secured.

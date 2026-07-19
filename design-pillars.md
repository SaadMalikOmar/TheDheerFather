# Core Systems

## PUD Level — "Player Under Death" (cowardice)

PUD is how much of a **coward** a character is — how badly they break down when the killer is
near. It's the game's signature fear mechanic.

- **Range:** 0–100.
- **What it controls** (scales with PUD value): heavy breathing, degraded hearing, eerie
  noises, fog, screen distortion, dizziness.
- **Trigger:** activates when the killer uses **scare tactics** (or simply gets close).
- **Downside of high PUD:** high-PUD runners are *louder* (breathing/heartrate) — which is how
  Skinny Bear and other killers can sense them even while still. Cowardice literally gets you caught.
- **Lowering PUD:** Mahnam's narcotics drop a player's PUD to 0 (see [runners.md](runners.md)).

> **Design intent:** PUD makes the same chase feel different per character. Julius (PUD 0) plays
> it cool; Dheer (PUD 90) is barely functional under pressure. This is the main reason to pick
> one runner over another beyond raw stats.

### Open question — PUD ↔ stealth interaction
High PUD = louder = easier for killers to detect. Confirm the exact relationship:
- Does PUD raise your **detection radius** to killers? (Recommended: yes, linearly.)
- Does it affect **movement** (Dheer's hands shaking, slower tasks) or only audio/visual?

## Kealth (Knockout-Health)

**Kealth** = the amount of health a character has before being **briefly knocked out** (not killed).
Used by killers (e.g. Skinny Bear Kealth 350). Clarify whether runners use Health, Kealth, or both:

- **Recommendation:** Runners use **Health** (down/injure on hit, can be finished/rescued).
  Killers use **Kealth** (can be stunned/knocked out but not killed) so runner weapons *delay*
  rather than kill.

## Stillness (anti-Skinny-Bear)

Skinny Bear can't see still players. For this to be fair and not frustrating:

- Runners need a dedicated **"go still / hold breath"** action — fast to enter, holds the player
  frozen, suppresses footstep/breathing audio.
- High-PUD runners should be **harder to hide** (breathing leaks through) → ties stillness to PUD.
- Tasks force movement, creating the core tension: *freeze and survive, or move and risk it.*

## Scare tactics

Killer abilities that spike runner PUD (voice mimicry, screams, jump-spawns like Lucki's Anju
door trap). Each killer should have at least one. Define a global rule for how much PUD a scare
adds and how fast it decays.

## OPEN QUESTIONS (resolve these next)

1. **Can runners damage/kill killers, or only stun?**
   Weapons list damage numbers (67/70) but it's unclear what they hit. Recommended: weapons
   **stun/knock out** killers (via Kealth) to buy time — they don't kill.
2. **Taser scope:** "only works on human killers" → only **Lucki** is taseable. Skinny Bear and
   Tung Tung are explicitly immune ("can't get tased / not human"). Confirm this is intended —
   it means the ranged tool is useless vs 2 of 3 killers.
3. **Down vs death:** when a runner hits 0 Health, are they downed (rescuable) or dead? Several
   kits assume rescue (DJ drag, Skinny Bear 3s animation) — define the universal down/rescue rule.
4. **"Player Con" weaknesses** (Dheer is Lucki's son → 5× longer to kill; Skinny Bear distorted
   near Mahnam's stink) create lobby-composition swings. Keep them as flavor perks but make sure
   no single matchup is degenerate.
5. **Rescue windows:** define how long teammates have during each killer's finisher
   (Skinny Bear 3s rip; Lucki finisher one-tap; DJ leg-drag).

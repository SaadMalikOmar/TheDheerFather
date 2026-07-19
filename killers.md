# Killers

Three killers, each a distinct **tempo/playstyle**. Stat blocks use the normalized speed scale
from [README.md](README.md) (raw original values preserved in notes).

---

## Mimic — Skinny Bear

> A mutated bear fused with many other creatures (think the bear from *Annihilation*).

**Concept:** The hard part is *finding* players — you can't see them while they're still. But
once you find one, it's a ~99% kill with no escape. Even high-heartrate / high-PUD players must
be in fairly close proximity to be heard. Players can tell he's near: the screen distorts from
his lingering stink.

| Stat | Value |
|------|-------|
| Stamina | 200 |
| Speed | **190** *(rebalanced down from 250 — still the fastest thing alive)* |
| Kealth | 350 |
| Damage | Infinite — **1 shot, 1 kill** |
| Fire rate | Kill is brutal: 3s of ripping face/chest/legs |

- **Passive:** Increased sprint speed; **can't be tased**.
- **Tactical:** Mimic player voices.
- **Ultimate:** Scream echolocation — reveals everyone's location.
- **Pros:** One-shot kills.
- **Cons:** Can't see **still** players. Can sense high-PUD players via breathing/heartrate when close.
- **Player Con:** Distorted (weakened) when close to **Mahnam** due to her stink.

**Balance notes:**
- Only counter is **stillness** → the global "go still" action must be reliable and fast.
- The **3s kill animation** is the rescue window — confirm teammates can interrupt it.
- Un-taseable + 250 speed + one-shot is oppressive; the stillness counter and Mahnam-distortion
  are the pressure-release valves. Make sure objectives don't force runners into long open sprints.

---

## Stalker — Lucki

> Fat Indian uncle; cannibal. Eats players alive — killing fuels him.

**Concept:** Keep your distance and keep running. His car drives around fast — when you hear it
coming, run. He places traps to find you; when **DJ** (his dog) bites you, one tap activates
Lucki's finisher.

| Stat | Value |
|------|-------|
| Stamina | 50 |
| Speed (hungry) | 5 |
| Speed (normal) | 50 |
| Speed (Diet Coke) | 150 |
| Kealth | 150 |
| Damage | 49 *(strong slow punches; 90 on a ram/2-shot — see below)* |
| Fire rate | Strong, slow punches |

- **Item:** Blue Prius — he's fat and slow, so he drives. Can ram & one-shot. **Hybrid:** silent
  on electric at low speed; can hide inside by turning lights off.
- **Passive:** Hunger economy — must **eat** or speed decays (food found in houses/stores).
- **Tactical (choose/loadout):**
  - **Spirit of Nani** — places a Ring doorbell that reveals location.
  - **Go invisible.**
  - **Anju door trap** — a demonic Indian lady spawns around corners / at doors and stabs you with a TV remote.
- **Ultimate:** **DJ** — rabid dog bites a runner's leg, injuring & dragging them down for Lucki
  to finish. **Takes 2 players to kill DJ.** Teammates can free the victim, but must flee if Lucki
  is coming (he 2-shots healthy players — **90 dmg**). Also: **3 Diet Cokes** = speed boosts.
- **Pros:** Prius for long-range travel; run over players for one-shots; silent slow-drive ambush.
- **Cons:** Slow on foot; constantly needs to eat to keep speed; spawns with only 3 Diet Cokes (find more).
- **Player Con:** **Dheer is his son** → takes **5× longer** to kill him.

**Balance notes:**
- *Damage inconsistency:* punch is 49, but DJ-context says he "2-shots healthy players (90 dmg)."
  Reconcile: is 90 the **ram/car** damage or a charged punch? Recommend: punch 49, car ram = down/one-shot.
- Car on the half-mile straight road is strong — ensure houses/forest break line of sight so the
  road isn't a pure death lane.

---

## Combatant — Tung Tung

> A mannequin possessed by an old man who performed black magic — wished to be reborn, was reborn
> as a tree, which was chopped and crafted into a mannequin. Now craves youth; kidnaps and wears
> the skins of his victims to become human again. Nicknamed "Tung Tung" by the teens.

**Concept:** The only killer with **FPS-style movement** — slide-cancel, a crosshair, skill-based
combat. His goal is not to simply kill but to **kidnap** all runners, drag them to his **shrine**,
and harvest their bodies. Each victim adds a human part to his wooden frame. Uniquely: he must
*carry* victims, which slows him down and makes him vulnerable.

| Stat | Value |
|------|-------|
| Stamina | *(TBD — suggest 150 to reflect agility)* |
| Speed | *(TBD — suggest 120: faster than most runners, but slows per carried victim)* |
| Kealth | *(TBD — suggest 200)* |
| Damage (bat swing) | *(TBD — suggest 60)* |
| Fire rate | Charged swings — slow but stun-radius on ground smash |

- **Items:**
  1. **Wooden Bat** — **ground smash**: stuns all enemies in a radius. **Charged swing**: direct hit.
  2. **Blow Dart (bamboo stick)** — tranquilizes a runner so he can carry them. Victim's arms are
     tied to his legs while being dragged → **he slows down per victim** (can carry max **2 at once**).
     Needs **all** runners on the shrine to win.

- **Passive:** Agile; **fits through any gap / any space**; **can't be tased** (not human — he's wood).

- **Tactical:** **Ground smash** — slam the bat into the floor, stunning enemies in a radius.
  *Recharges every 30 seconds.* *(In Hardcore mode: limited uses — see [difficulties.md](difficulties.md).)*

- **Ultimate: Root of Flesh** *(lasts 10 seconds)*
  Tung Tung transforms into a **tall, super-slender creature** — think extreme limb elongation.
  In this form he can:
  - **Swing through trees** and traverse the forest canopy at extreme speed.
  - **Grab runners** and pull them up into the trees — jump-scaring them — then **stab them through
    the chest and pin them to the tree.**
  - Each follow-up attack takes **4 seconds to charge.**
  - A pinned player can only be **freed by any runner who can climb** (everyone except **Troos**).
  - A pinned player can only be **brought down safely by the Medic (Mahnam)**.

- **Pros:** Skill-ceiling combat; fastest traversal in forest; only killer with a hard **win condition**
  that isn't just kills.
- **Cons:** Carrying victims slows him significantly; shrine-based win means runners can contest it;
  10s Ultimate window is short if no one's in the forest.

### Body-part progression (per kill)

| Kill # | Part added | Suggested buff |
|:------:|-----------|----------------|
| 1 | Victim's **face** | *(cosmetic — uncanny horror effect)* |
| 2 | **Right arm skin** | Swing deals slightly more damage |
| 3 | **Left arm skin** | Blow dart range increases |
| 4 | **Legs** | Slight speed increase |
| 5+ | **Full upper body** | — |

> Progression is cosmetic by default; buff column is a suggested option to make the early-game
> race to first kill feel rewarding. Confirm whether buffs are in scope.

### Shrine / win condition rules
- Carrying 2 victims simultaneously slows Tung Tung significantly.
- Victims can be **freed** from the shrine by other runners — define how long that takes and
  whether Tung Tung is notified.
- Recommend: a **shrine alert** pings him when a victim is being freed, same way survivors get
  a generator-pop noise in DbD — forces him to patrol.

**Balance notes:**
- Tung Tung is the only killer who **loses if runners free shrine victims fast enough**, making
  him the most counter-playable killer. His power floor is lower but his ceiling (skilled player
  in forest with Root of Flesh) is terrifying.
- Speed-while-carrying needs careful tuning: too slow = he never gets victims to the shrine; too
  fast = no counter-play window for the rescue.

---

## ✅ Implemented kits (current build)

All killers wait through Stage 1 — **no attacking until the blackout** (`KillersUnleashed`).

### Skinny Bear (SPD 190) — the ONLY third-person character
- Still runners are invisible on his screen (velocity threshold). One-shot melee (LMB).
- **RMB / E — Sniff:** half speed for 5s, but reveals everyone within 50 m **through walls** for 6s.
- **R — Echo Scream** (reveal all). *(Mahnam stink-distortion cut with the Emma rework.)*

### Lucki (SPD 70 on foot) — the Prius & DJ
- **The Prius is a real parked car:** walk to it → **E** sit in → **E** keys in → **RMB** crank
  (5s) → drive (300) → **Shift** out (re-parks). **RMB looking at it** = lock/unlock.
- **Hybrid fuel:** fast = petrol (charges battery), slow = silent electric; dry = furniture.
  Refuel at **MOST HATED S.O** stations (engine off) — Whyteleafe has NONE on purpose.
- **Hunger** drains; feed on downed runners or loot-house food. Starving = speed 5.
- **Q** Nani trap ×4 · **T** Diet Coke ×3 · **R** release DJ · ram kills at car speed.
- **DJ modes (RMB whistle cycles): SCOUT** (tracks & pings runners to Lucki, no bite) /
  **ATTACK** (lone wolf, baitable & killable) / **DEFEND** (shadows Lucki, mauls anyone within 30 m).

### Tung Tung (SPD 120) — the shrine
- **E** carry stunned/downed runners → tie to shrine (all runners tied/dead = win). Auto-drop on stun.
- **T — Tree Form:** becomes a tree with **one red leaf**; 60s recharge, free exit, breaks on stun.
- **RMB** blow dart · **Q** ground smash · **R — Root of Flesh** locked until **3 min** after spawn.
- Body-part progression: not yet implemented (cosmetics pass).

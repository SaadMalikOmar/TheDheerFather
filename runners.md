# Runners — The 6 Base Characters

Roles: **Scout, Tank, Engineer, Trickster, Stealth, Medic.** Speeds shown on the normalized scale
(`100` = average human jog; baseline runner = `60`). See [README.md](README.md).

| Role | Character | Health | Stamina | Speed | PUD | One-line identity |
|------|-----------|:------:|:-------:|:-----:|:---:|-------------------|
| Scout | Dheer | 100 | 100 | 60 | 90 | Smart watch (hold Q): heartbeat + generator sense |
| Tank | Troos | 200 | 30 | 55 | 30 | Huge & tanky, trap-proof — but can't climb trees |
| Engineer | Julius | 75 | 75 | 70 | 0 | 3× repair speed, but poor vision |
| Trickster | Musa | 100 | 50 | 50 | 25/100 | Decoys + hidden voice-note speakers, phone-addicted |
| Stealth | Shaun | 60 | 100 | 100 | 25 | Near-invisible when still + holding breath; dies in ONE hit |
| Medic | Emma | 75 | 75 | 70 | 58 | White emo raver — her stash heals & drops PUD to 0 |

**Global mechanic — SUPER RUN:** keep your stamina bar full for a full minute → bank a charge
(max 3). Your next sprint drains at **half rate** (one double-length run per charge).
**Tree climbing:** hold Jump at a trunk to climb (4 stam/s); add **Ctrl to hang on** (0.8 stam/s).

---

## Scout — Dheer
*Indian, 22, boxing attire + mouth guard.*

- **Pro:** **Heartbeat sensor** (20s battery) — detect the killer's proximity.
- **Cons:** Tasks take **3× longer**; **5% chance** a task registers as done but is actually wrong
  (the game *shows* it complete) → at the final exit it reveals as "1 task left."
- **Player Con:** **Lucki's son** → Lucki takes 5× longer to kill him.

| Health | Stamina | Speed | PUD |
|:------:|:-------:|:-----:|:---:|
| 100 | 100 | 60 | 90 |

> Note: PUD 90 = extreme coward. Pair with the highest fear distortion. The heartbeat sensor is
> his way to manage that fear with information.

---

## Tank — Troos
*Fat Indian guy with glasses.*

- **Pros:** Immune to **all Lucki traps except the camera**; takes **Skinny Bear 10s** to kill him
  (vs the normal 3s) because he's so big.
- **Cons:** Slow; **gets stuck in trees**; **can't climb**; if he **trips** he gets injured.

| Health | Stamina | Speed | PUD |
|:------:|:-------:|:-----:|:---:|
| 200 | 30 | 40 | 30 |

---

## Engineer — Julius
*Chinese.*

- **Pros:** **3× repairing** speed on objects.
- **Cons:** **Reduced vision** — must **hold Q** to pry eyes wide open to see properly.

| Health | Stamina | Speed | PUD |
|:------:|:-------:|:-----:|:---:|
| 75 | 75 | 70 | 0 |

> PUD 0 = fearless. The objective specialist who can work under pressure.

---

## Trickster — Musa
*Stubby chubby Pakistani boy.*

- **Pros:** Sets **traps** and **dummies / fake footsteps**.
- **Cons:** **Phone addict** — every **30s** must scroll IG Reels for **5s** to be able to move.
  A nearby teammate can snap him out of it by telling him to **"lock in."**

| Health | Stamina | Speed | PUD (near his traps) | PUD (normal) |
|:------:|:-------:|:-----:|:---:|:---:|
| 100 | 50 | 50 | 25 | 100 |

> Dynamic PUD: brave near his own traps (home turf), terrified otherwise.

---

## Stealth — Shaun Elijah Koudou Daddie
*Very dark-skinned; near-invisible in shadow.*

- **Pros:** Quieter footsteps. In the dark, closing eyes & mouth makes him **effectively invisible**
  — implement as: fully invisible until a killer is **close**, then he's just a very dark
  **silhouette**.
- **Cons:** If caught, **dies instantly** (no down).

| Health | Stamina | Speed | PUD |
|:------:|:-------:|:-----:|:---:|
| 60 | 100 | 100 | 25 |

> High-risk/high-reward: fastest & stealthiest, but lowest effective survivability if spotted.

---

## Medic — Emma
*White emo girl, total rave head, walking pharmacy.*

- **Pros:** **Q — the rave stash**: heals players with her narcotics/peptides and drops their
  **PUD to 0** (chemically fearless).
- **Cons:** Fragile; her whole kit is consumables (limited uses in Hardcore).

| Health | Stamina | Speed | PUD |
|:------:|:-------:|:-----:|:---:|
| 75 | 75 | 70 | 58 |

> Replaced the old "Mahnam" design — the stink aura / revive-restriction mechanics were cut.

---

## Implemented kit notes (current build)

- **Dheer:** HOLD **Q** = smart watch. Head locks to his wrist; battery (20s) shows killer
  heartbeat distance; **generator sense works forever**, even at 0 battery, but only while held.
- **Troos:** cannot climb trees (`CanClimbTrees() = false`).
- **Musa:** **hold RMB** records a voice note, release plants an invisible speaker (×3) that plays
  phantom voices at the killer. Q = decoy. "Lock in" rescue by teammates works.
- **Shaun:** invisible while still + holding breath (**F**); one hit = dead.
- **Stage 2:** everyone needs a **repair kit** (spawns in your house) to fix generators, and a
  2nd runner standing lookout.

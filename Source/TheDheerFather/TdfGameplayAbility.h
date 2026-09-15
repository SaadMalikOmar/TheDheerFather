// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TdfGameplayAbility.generated.h"

/** Base class for all of the game's abilities (killer powers, runner utilities). */
UCLASS(Abstract)
class THEDHEERFATHER_API UTdfGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UTdfGameplayAbility();

	/** In HARDCORE this ability has a fixed number of uses (no recharge). -1 = unlimited. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Hardcore")
	int32 HardcoreUseLimit = -1;

	// --- HUD mirrors (read by the loadout panel; updated whenever the ability runs) ---

	/** World time the current cooldown ends (0 = never used). */
	float HudCooldownEnd = 0.f;

	/** Length of the last-started cooldown (for the charge-up bar). */
	float HudCooldownDuration = 0.f;

	/** Uses remaining for limited abilities; -1 = not limited / unknown yet. */
	int32 HudUsesLeft = -1;

protected:
	/** Gate for hardcore's fixed-use pools. Returns false (and warns) when the pool is dry. */
	bool ConsumeHardcoreUse(const AActor* Avatar);

	/** Shared manual-cooldown helpers (also feed the HUD loadout panel). */
	bool IsOnCooldown(const APawn* Avatar, float& CooldownEndTime) const;
	void StartCooldown(const APawn* Avatar, float& CooldownEndTime, float Cooldown);

private:
	int32 HardcoreUsesSoFar = 0;
};

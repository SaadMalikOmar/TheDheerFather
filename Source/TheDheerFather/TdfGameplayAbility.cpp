#include "TdfGameplayAbility.h"
#include "TdfCharacterBase.h"
#include "TdfTypes.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

UTdfGameplayAbility::UTdfGameplayAbility()
{
	// One instance per actor: lets the ability hold per-character state between activations.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UTdfGameplayAbility::ConsumeHardcoreUse(const AActor* Avatar)
{
	const ATdfCharacterBase* Character = Cast<ATdfCharacterBase>(Avatar);
	if (!Character || HardcoreUseLimit < 0 || Character->GetMatchDifficulty() != ETdfDifficulty::Hardcore)
	{
		return true;
	}
	if (HardcoreUsesSoFar >= HardcoreUseLimit)
	{
		const APawn* Pawn = Cast<APawn>(Avatar);
		if (GEngine && Pawn && Pawn->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("No uses left (Hardcore)"));
		}
		return false;
	}
	++HardcoreUsesSoFar;
	HudUsesLeft = HardcoreUseLimit - HardcoreUsesSoFar;
	return true;
}

bool UTdfGameplayAbility::IsOnCooldown(const APawn* Avatar, float& CooldownEndTime) const
{
	return Avatar && Avatar->GetWorld() && Avatar->GetWorld()->GetTimeSeconds() < CooldownEndTime;
}

void UTdfGameplayAbility::StartCooldown(const APawn* Avatar, float& CooldownEndTime, float Cooldown)
{
	if (Avatar && Avatar->GetWorld())
	{
		CooldownEndTime = Avatar->GetWorld()->GetTimeSeconds() + Cooldown;
		HudCooldownEnd = CooldownEndTime;
		HudCooldownDuration = Cooldown;
	}
}

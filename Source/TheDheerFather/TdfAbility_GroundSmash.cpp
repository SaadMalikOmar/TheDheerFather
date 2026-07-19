#include "TdfAbility_GroundSmash.h"
#include "TdfCharacterBase.h"
#include "TdfGameState.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UTdfAbility_GroundSmash::UTdfAbility_GroundSmash()
{
	HardcoreUseLimit = 5;
}

void UTdfAbility_GroundSmash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Avatar || !World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Simple cooldown gate.
	const float Now = World->GetTimeSeconds();
	if (Now < CooldownEndTime)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver,
				FString::Printf(TEXT("Ground Smash on cooldown (%.1fs)"), CooldownEndTime - Now));
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ConsumeHardcoreUse(Avatar) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CooldownEndTime = Now + Cooldown;
	HudCooldownEnd = CooldownEndTime;
	HudCooldownDuration = Cooldown;

	// AoE: stun every Tdf character within Radius. Stuns are applied on the SERVER so they replicate.
	const FVector Origin = Avatar->GetActorLocation();
	int32 StunnedCount = 0;
	if (Avatar->HasAuthority() && ATdfGameState::KillersUnleashed(World))
	{
		TArray<AActor*> Candidates;
		UGameplayStatics::GetAllActorsOfClass(World, ATdfCharacterBase::StaticClass(), Candidates);

		for (AActor* Actor : Candidates)
		{
			if (Actor == Avatar)
			{
				continue;
			}
			if (FVector::Dist(Origin, Actor->GetActorLocation()) <= Radius)
			{
				if (ATdfCharacterBase* Target = Cast<ATdfCharacterBase>(Actor))
				{
					Target->ApplyStun(StunDuration);
					++StunnedCount;
				}
			}
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple,
			FString::Printf(TEXT("GROUND SMASH! Stunned %d"), StunnedCount));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

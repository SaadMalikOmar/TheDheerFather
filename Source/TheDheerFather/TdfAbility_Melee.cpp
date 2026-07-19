#include "TdfAbility_Melee.h"
#include "TdfCharacterBase.h"
#include "TdfRunnerCharacter.h"
#include "TdfKillerCharacter.h"
#include "TdfKillers.h"
#include "TdfAttributeSet.h"
#include "TdfGameState.h"
#include "TdfGE_Damage.h"
#include "TdfGE_KealthDamage.h"
#include "TdfTypes.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UTdfAbility_Melee::UTdfAbility_Melee()
{
	HealthDamageEffect = UTdfGE_Damage::StaticClass();
	KealthDamageEffect = UTdfGE_KealthDamage::StaticClass();
}

void UTdfAbility_Melee::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;

	// Swing rate limit.
	if (Avatar && Avatar->GetWorld())
	{
		const float Now = Avatar->GetWorld()->GetTimeSeconds();
		if (Now < CooldownEndTime)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		CooldownEndTime = Now + SwingCooldown;
		HudCooldownEnd = CooldownEndTime;
		HudCooldownDuration = SwingCooldown;
	}

	if (Avatar && Avatar->GetWorld())
	{
		// The taser is the one ranged runner weapon.
		float Range = TraceRange;
		if (const ATdfRunnerCharacter* WeaponCheck = Cast<ATdfRunnerCharacter>(Avatar))
		{
			if (WeaponCheck->CurrentWeapon == ETdfWeaponType::Taser)
			{
				Range = 1200.f;
			}
		}
		const FVector Start = Avatar->GetPawnViewLocation();
		const FVector End = Start + Avatar->GetControlRotation().Vector() * Range;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Avatar);

		const bool bHit = Avatar->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params);
		ATdfCharacterBase* Target = bHit ? Cast<ATdfCharacterBase>(Hit.GetActor()) : nullptr;

		// Damage and kill decisions are SERVER-ONLY so they replicate to everyone.
		if (Target && Avatar->HasAuthority())
		{
			UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
			ATdfKillerCharacter* AvatarKiller = Cast<ATdfKillerCharacter>(Avatar);

			// Stage 1: the killer stays his hand until the power dies.
			if (AvatarKiller && !ATdfGameState::KillersUnleashed(Avatar->GetWorld()))
			{
				if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("Not yet... wait for the power to die.")); }
				EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
				return;
			}
			ATdfKillerCharacter* TargetKiller = Cast<ATdfKillerCharacter>(Target);
			ATdfRunnerCharacter* TargetRunner = Cast<ATdfRunnerCharacter>(Target);

			if (TargetKiller)
			{
				// Runner attacking the killer — outcome depends on the weapon in hand.
				ATdfRunnerCharacter* AvatarRunner = Cast<ATdfRunnerCharacter>(Avatar);
				if (Target->GetMatchDifficulty() == ETdfDifficulty::Hardcore)
				{
					if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("The killer shrugs it off (Hardcore)")); }
				}
				else if (AvatarRunner)
				{
					switch (AvatarRunner->CurrentWeapon)
					{
					case ETdfWeaponType::Axon:
						ApplyDamageTo(SourceASC, Target, KealthDamageEffect, 67.f);
						TargetKiller->StartBleed(4, 5.f); // bleed out
						AvatarRunner->ConsumeWeaponHit();
						break;
					case ETdfWeaponType::TripleTBat:
						ApplyDamageTo(SourceASC, Target, KealthDamageEffect, 70.f);
						TargetKiller->ApplyStun(2.f);
						AvatarRunner->ConsumeWeaponHit();
						break;
					case ETdfWeaponType::Taser:
						// Only works on human killers — that means Lucki.
						if (TargetKiller->KillerType == ETdfKillerType::Stalker_Lucki)
						{
							ApplyDamageTo(SourceASC, Target, KealthDamageEffect, 10.f);
							TargetKiller->ApplyStun(3.f);
						}
						else if (GEngine)
						{
							GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("The taser fizzles - that thing isn't human"));
						}
						AvatarRunner->ConsumeWeaponHit(); // one zap either way
						break;
					default:
						if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("Punching it does nothing - find a weapon!")); }
						break;
					}
				}
				else
				{
					ApplyDamageTo(SourceASC, Target, KealthDamageEffect, RunnerWeaponDamage);
				}
			}
			else if (TargetRunner)
			{
				const ATdfKiller_TungTung* TungTung = Cast<ATdfKiller_TungTung>(Avatar);
				if (TargetRunner->bIsDown)
				{
					// Finisher on a downed runner.
					TargetRunner->Die();
				}
				else if (AvatarKiller && TargetRunner->bDiesInstantly)
				{
					// Shaun's curse: if a killer catches him, he dies outright.
					TargetRunner->Die();
				}
				else if (TungTung && TungTung->bRootOfFleshActive)
				{
					// Root of Flesh: one touch pins them.
					if (UTdfAttributeSet* Attr = TargetRunner->GetTdfAttributeSet())
					{
						Attr->SetHealth(0.f);
					}
					TargetRunner->OnHealthDepleted();
				}
				else
				{
					const float Damage = AvatarKiller ? AvatarKiller->Stats.Damage : RunnerWeaponDamage;
					ApplyDamageTo(SourceASC, Target, HealthDamageEffect, Damage);
				}
			}
			else
			{
				// Dummy / anything else with an ASC.
				ApplyDamageTo(SourceASC, Target, HealthDamageEffect, RunnerWeaponDamage);
			}
		}

		// Local swing feedback.
		if (GEngine && Avatar->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, bHit ? FColor::Red : FColor::White,
				bHit && Hit.GetActor() ? *FString::Printf(TEXT("Hit: %s"), *Hit.GetActor()->GetName()) : TEXT("Swing - missed"));
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTdfAbility_Melee::ApplyDamageTo(UAbilitySystemComponent* SourceASC, ATdfCharacterBase* Target,
	TSubclassOf<UGameplayEffect> Effect, float Damage) const
{
	UAbilitySystemComponent* TargetASC = Target ? Target->GetAbilitySystemComponent() : nullptr;
	if (!SourceASC || !TargetASC || !Effect)
	{
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Effect, GetAbilityLevel(), Context);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), -Damage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

#include "TdfCharacterAbilities.h"
#include "TdfRunnerCharacter.h"
#include "TdfKillers.h"
#include "TdfAttributeSet.h"
#include "TdfDeployables.h"
#include "TdfDJDog.h"
#include "TdfGameState.h"
#include "TdfGE_SpeedBoost.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

// (IsOnCooldown / StartCooldown moved into UTdfGameplayAbility so the HUD loadout
//  panel can read cooldown state — same call syntax, they're base-class members now.)

// --- Heartbeat Sensor (Dheer) ---

void UTdfAbility_HeartbeatSensor::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (ATdfRunnerCharacter* Runner = Cast<ATdfRunnerCharacter>(Avatar))
	{
		Runner->SensorTimeRemaining = SensorDuration;
		if (GEngine && Runner->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Heartbeat sensor ON (20s)"));
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Heal (Mahnam) ---

void UTdfAbility_Heal::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (Avatar && Avatar->HasAuthority())
	{
		for (TActorIterator<ATdfRunnerCharacter> It(Avatar->GetWorld()); It; ++It)
		{
			ATdfRunnerCharacter* Runner = *It;
			if (Runner && !Runner->bIsDown && !Runner->IsDead()
				&& FVector::Dist(Runner->GetActorLocation(), Avatar->GetActorLocation()) <= HealRadius)
			{
				if (UTdfAttributeSet* Attr = Runner->GetTdfAttributeSet())
				{
					Attr->SetHealth(FMath::Min(Attr->GetMaxHealth(), Attr->GetHealth() + HealAmount));
					Attr->SetPud(0.f); // her narcotics calm you right down
				}
			}
		}
	}

	if (GEngine && Avatar && Avatar->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Narcotics administered: healed + PUD to 0"));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Place Decoy (Musa) ---

void UTdfAbility_PlaceDecoy::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (Avatar && Avatar->HasAuthority())
	{
		const FVector Location = Avatar->GetActorLocation() - Avatar->GetActorForwardVector() * 120.f;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Avatar->GetWorld()->SpawnActor<ATdfDecoy>(ATdfDecoy::StaticClass(), Location, Avatar->GetActorRotation(), Params);
	}

	if (GEngine && Avatar && Avatar->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Decoy placed"));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Diet Coke (Lucki) ---

UTdfAbility_DietCoke::UTdfAbility_DietCoke()
{
	SpeedBoostEffect = UTdfGE_SpeedBoost::StaticClass();
}

void UTdfAbility_DietCoke::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (ActorInfo && SpeedBoostEffect)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SpeedBoostEffect, GetAbilityLevel(), Context);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	if (GEngine && Avatar && Avatar->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("DIET COKE! *crack* *glug*"));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Spirit of Nani (Lucki) ---

void UTdfAbility_PlaceNaniTrap::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (UsesLeft <= 0)
	{
		if (GEngine && Avatar && Avatar->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("Out of Nanis"));
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	--UsesLeft;
	HudUsesLeft = UsesLeft;

	if (Avatar && Avatar->HasAuthority())
	{
		const FVector Location = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 150.f;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Avatar->GetWorld()->SpawnActor<ATdfNaniTrap>(ATdfNaniTrap::StaticClass(), Location, FRotator::ZeroRotator, Params);
	}

	if (GEngine && Avatar && Avatar->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
			FString::Printf(TEXT("Nani deployed (%d left)"), UsesLeft));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Echo Scream (Skinny Bear) ---

void UTdfAbility_EchoScream::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !ConsumeHardcoreUse(Avatar) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (ATdfKiller_SkinnyBear* Bear = Cast<ATdfKiller_SkinnyBear>(Avatar))
	{
		Bear->RevealTimeRemaining = RevealDuration;

		// Ping every runner's position on the killer's screen.
		if (Bear->IsLocallyControlled())
		{
			for (TActorIterator<ATdfRunnerCharacter> It(Bear->GetWorld()); It; ++It)
			{
				if (const ATdfRunnerCharacter* Runner = *It)
				{
					if (!Runner->IsDead() && !Runner->HasEscaped())
					{
						DrawDebugSphere(Bear->GetWorld(), Runner->GetActorLocation(), 60.f, 12, FColor::Red, false, RevealDuration);
					}
				}
			}
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("SCREEEEEAM - everyone revealed"));
			}
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Release DJ (Lucki) ---

void UTdfAbility_ReleaseDJ::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// One DJ at a time.
	if (Avatar && Avatar->GetWorld())
	{
		for (TActorIterator<ATdfDJDog> It(Avatar->GetWorld()); It; ++It)
		{
			if (*It && !(*It)->bIsDown)
			{
				if (GEngine && Avatar->IsLocallyControlled())
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("DJ is already out"));
				}
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}
		}
	}

	if (!ConsumeHardcoreUse(Avatar) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (Avatar && Avatar->HasAuthority())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Params.Owner = Avatar; // DJ knows who his master is (heel mode follows the owner)
		Avatar->GetWorld()->SpawnActor<ATdfDJDog>(ATdfDJDog::StaticClass(),
			Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 200.f, Avatar->GetActorRotation(), Params);
	}
	if (GEngine && Avatar && Avatar->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("GO GET 'EM, DJ!"));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Root of Flesh (Tung Tung) ---

void UTdfAbility_RootOfFlesh::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !ConsumeHardcoreUse(Avatar) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (ATdfKiller_TungTung* TungTung = Cast<ATdfKiller_TungTung>(Avatar))
	{
		// The ultimate needs 3 minutes of growth after spawning.
		const float Age = TungTung->GetGameTimeSinceCreation();
		if (Age < TungTung->UltimateUnlockSeconds)
		{
			CooldownEndTime = 0.f; // don't burn the cooldown on an early press
			if (GEngine && TungTung->IsLocallyControlled())
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver,
					FString::Printf(TEXT("Root of Flesh not ready (%.0fs)"), TungTung->UltimateUnlockSeconds - Age));
			}
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		if (TungTung->HasAuthority())
		{
			TungTung->StartRootOfFlesh(Duration);
		}
		if (GEngine && TungTung->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Purple, TEXT("ROOT OF FLESH - 10s of nightmare"));
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Sniff (Skinny Bear) ---

void UTdfAbility_Sniff::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (ATdfKiller_SkinnyBear* Bear = Cast<ATdfKiller_SkinnyBear>(Avatar))
	{
		if (Bear->HasAuthority())
		{
			Bear->StartSniffSlow(SlowDuration);
		}
		// Lingering smells become sight: still runners show, and everyone nearby glows through walls.
		Bear->RevealTimeRemaining = FMath::Max(Bear->RevealTimeRemaining, RevealDuration);

		if (Bear->IsLocallyControlled())
		{
			for (TActorIterator<ATdfRunnerCharacter> It(Bear->GetWorld()); It; ++It)
			{
				const ATdfRunnerCharacter* Runner = *It;
				if (Runner && !Runner->IsDead() && !Runner->HasEscaped()
					&& FVector::Dist(Runner->GetActorLocation(), Bear->GetActorLocation()) <= SniffRadius)
				{
					DrawDebugSphere(Bear->GetWorld(), Runner->GetActorLocation(), 55.f, 10,
						FColor::Yellow, false, RevealDuration, SDPG_Foreground, 2.f);
				}
			}
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("*sniiiiff* ... he smells them"));
			}
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Tree Form (Tung Tung) ---

void UTdfAbility_TreeForm::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	ATdfKiller_TungTung* TungTung = Cast<ATdfKiller_TungTung>(Avatar);
	if (!TungTung)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Dropping the disguise is always free; putting it on costs the 60s recharge.
	if (TungTung->bTreeForm)
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		if (TungTung->HasAuthority())
		{
			TungTung->SetTreeForm(false);
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (IsOnCooldown(Avatar, CooldownEndTime) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (TungTung->HasAuthority())
	{
		TungTung->SetTreeForm(true);
	}
	if (GEngine && TungTung->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("You are a tree now. Mind the red leaf."));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Place Speaker (Musa) ---

void UTdfAbility_PlaceSpeaker::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (UsesLeft <= 0)
	{
		if (GEngine && Avatar && Avatar->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver, TEXT("Out of speakers"));
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	--UsesLeft;
	HudUsesLeft = UsesLeft;

	if (Avatar && Avatar->HasAuthority())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Avatar->GetWorld()->SpawnActor<ATdfSpeaker>(ATdfSpeaker::StaticClass(),
			Avatar->GetActorLocation(), Avatar->GetActorRotation(), Params);
	}
	if (GEngine && Avatar && Avatar->IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			FString::Printf(TEXT("Speaker hidden here (%d left)"), UsesLeft));
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// --- Blow Dart (Tung Tung) ---

void UTdfAbility_BlowDart::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	APawn* Avatar = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (IsOnCooldown(Avatar, CooldownEndTime) || !ConsumeHardcoreUse(Avatar) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	StartCooldown(Avatar, CooldownEndTime, Cooldown);

	if (Avatar && Avatar->GetWorld())
	{
		const FVector Start = Avatar->GetPawnViewLocation();
		const FVector End = Start + Avatar->GetControlRotation().Vector() * DartRange;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Avatar);
		const bool bHit = Avatar->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params);

		if (bHit)
		{
			if (ATdfRunnerCharacter* Runner = Cast<ATdfRunnerCharacter>(Hit.GetActor()))
			{
				if (Avatar->HasAuthority() && ATdfGameState::KillersUnleashed(Avatar->GetWorld()))
				{
					Runner->ApplyStun(TranqDuration);
				}
				if (GEngine && Avatar->IsLocallyControlled())
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, TEXT("Dart hit - target tranquilized!"));
				}
			}
		}
		else if (GEngine && Avatar->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, TEXT("Dart missed"));
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

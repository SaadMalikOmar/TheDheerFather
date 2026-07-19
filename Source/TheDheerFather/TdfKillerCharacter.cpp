#include "TdfKillerCharacter.h"
#include "TdfAttributeSet.h"
#include "TdfAbility_Melee.h"
#include "TdfGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATdfKillerCharacter::ATdfKillerCharacter()
{
	// Kit slots: [1] = Q tactical (per killer), [2] = LMB melee, [3] = R power (per killer).
	DefaultAbilities.SetNum(6);
	DefaultAbilities[2] = UTdfAbility_Melee::StaticClass();
}

void ATdfKillerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentKealth = Stats.Kealth;
	ApplyMoveSpeed(Stats.Speed);
}

void ATdfKillerCharacter::InitializeAttributes()
{
	if (AttributeSet)
	{
		AttributeSet->SetMaxKealth(Stats.Kealth);
		AttributeSet->SetKealth(Stats.Kealth);
		AttributeSet->SetMaxStamina(Stats.Stamina);
		AttributeSet->SetStamina(Stats.Stamina);
		AttributeSet->SetMoveSpeed(Stats.Speed);
	}
}

float ATdfKillerCharacter::GetHealthBarValue() const { return AttributeSet ? AttributeSet->GetKealth() : 0.f; }
float ATdfKillerCharacter::GetHealthBarMax() const { return AttributeSet ? AttributeSet->GetMaxKealth() : 0.f; }
FString ATdfKillerCharacter::GetHealthBarLabel() const { return TEXT("Kealth"); }

void ATdfKillerCharacter::OnKealthDepleted()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	switch (GetMatchDifficulty())
	{
	case ETdfDifficulty::Casual:
		// Casual: the killer can be killed outright -> runners win.
		Die();
		if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
		{
			GameMode->EndMatch(true);
		}
		break;

	case ETdfDifficulty::Normal:
	default:
		// Normal: briefly knocked out, then back up at full Kealth.
		bIsDown = true;
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->DisableMovement();
		}
		if (GEngine && IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, KnockoutDuration, FColor::Orange, TEXT("KNOCKED OUT!"));
		}
		GetWorldTimerManager().SetTimer(RecoveryTimerHandle, this, &ATdfKillerCharacter::Recover, KnockoutDuration, false);
		break;

	case ETdfDifficulty::Hardcore:
		// Hardcore: weapons only distort — Kealth shouldn't reach 0, but recover instantly if it does.
		Recover();
		break;
	}
}

void ATdfKillerCharacter::Recover()
{
	Super::Recover();

	// Back up at full Kealth.
	if (!bIsDead && AttributeSet)
	{
		AttributeSet->SetKealth(AttributeSet->GetMaxKealth());
	}
}

void ATdfKillerCharacter::StartBleed(int32 Ticks, float DamagePerTick)
{
	if (!HasAuthority())
	{
		return;
	}
	BleedTicksLeft = Ticks;
	BleedDamagePerTick = DamagePerTick;
	GetWorldTimerManager().SetTimer(BleedTimerHandle, this, &ATdfKillerCharacter::BleedTick, 1.f, true);
}

void ATdfKillerCharacter::BleedTick()
{
	if (BleedTicksLeft <= 0 || bIsDead || !AttributeSet)
	{
		GetWorldTimerManager().ClearTimer(BleedTimerHandle);
		return;
	}
	--BleedTicksLeft;
	AttributeSet->SetKealth(FMath::Max(0.f, AttributeSet->GetKealth() - BleedDamagePerTick));
	if (AttributeSet->GetKealth() <= 0.f)
	{
		GetWorldTimerManager().ClearTimer(BleedTimerHandle);
		OnKealthDepleted();
	}
}

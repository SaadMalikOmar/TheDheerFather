#include "TdfTargetDummy.h"
#include "TdfAbilitySystemComponent.h"
#include "TdfAttributeSet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ATdfTargetDummy::ATdfTargetDummy()
{
	// Never controlled; just stands there. The visible body (BodyMesh) comes from the base class.
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;
}

void ATdfTargetDummy::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Never possessed by a controller, so initialise the ASC here rather than in PossessedBy.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (AttributeSet)
		{
			AttributeSet->SetMaxHealth(StartingHealth);
			AttributeSet->SetHealth(StartingHealth);
			AttributeSet->SetMaxKealth(StartingHealth);
			AttributeSet->SetKealth(StartingHealth);
		}

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UTdfAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ATdfTargetDummy::OnHealthChanged);
	}
}

void ATdfTargetDummy::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
			FString::Printf(TEXT("Dummy Health: %.0f"), Data.NewValue));
	}
}

void ATdfTargetDummy::OnHealthDepleted()
{
	Super::OnHealthDepleted();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("DUMMY DOWN!"));
	}

	// Topple the cylinder so the knockout is visible.
	if (BodyMesh)
	{
		BodyMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	}

	// Casual: killers can be killed outright -> stay down. Otherwise briefly knocked out, then recover.
	if (GetMatchDifficulty() == ETdfDifficulty::Casual)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("(Casual: stays down)"));
		}
		return;
	}

	GetWorldTimerManager().SetTimer(RecoveryTimerHandle, this, &ATdfTargetDummy::Recover, KnockoutDuration, false);
}

void ATdfTargetDummy::Recover()
{
	Super::Recover();

	// Stand the cylinder back up and refill health.
	if (BodyMesh)
	{
		BodyMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (AttributeSet)
	{
		AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("DUMMY RECOVERED"));
	}
}

void ATdfTargetDummy::ApplyStun(float Duration)
{
	Super::ApplyStun(Duration);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Cyan,
			FString::Printf(TEXT("DUMMY STUNNED (%.0fs)"), Duration));
	}

	// Tilt to one side to show the stun (only if not toppled/down).
	if (BodyMesh && !bIsDown)
	{
		BodyMesh->SetRelativeRotation(FRotator(0.f, 0.f, 25.f));
	}
}

void ATdfTargetDummy::RemoveStun()
{
	Super::RemoveStun();

	if (BodyMesh && !bIsDown)
	{
		BodyMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Dummy stun ended"));
	}
}

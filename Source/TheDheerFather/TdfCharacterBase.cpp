// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#include "TdfCharacterBase.h"
#include "TdfAbilitySystemComponent.h"
#include "TdfAttributeSet.h"
#include "TdfGameplayAbility.h"
#include "TdfGameMode.h"
#include "TdfHouseActors.h"
#include "TdfKillers.h"
#include "TdfRunnerCharacter.h"
#include "TdfDJDog.h"
#include "TdfDeployables.h"
#include "EngineUtils.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ATdfCharacterBase::ATdfCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// First-person camera mounted at eye height on the capsule.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	AbilitySystemComponent = CreateDefaultSubobject<UTdfAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UTdfAttributeSet>(TEXT("AttributeSet"));

	// Placeholder visible body (cylinder). Hidden from the owner so first-person stays clean.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetOwnerNoSee(true);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (BodyMeshAsset.Succeeded())
	{
		BodyMesh->SetStaticMesh(BodyMeshAsset.Object);
	}

	// The real character model goes on the inherited skeletal mesh (assign in a Blueprint subclass).
	// Hidden from the owner so first-person stays clean; other players see it animate.
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->SetOwnerNoSee(true);
		SkelMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
	}
}

void ATdfCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyMoveSpeed(MoveSpeed);

	if (FirstPersonCamera)
	{
		CameraBaseRelLoc = FirstPersonCamera->GetRelativeLocation();
	}

	// If a real character model is assigned, hide the placeholder cylinder.
	if (BodyMesh && GetMesh() && GetMesh()->GetSkeletalMeshAsset())
	{
		BodyMesh->SetVisibility(false);
	}
}

void ATdfCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsAsleep)
	{
		if (HasAuthority())
		{
			UpdateSleep(DeltaSeconds); // server drives the sleep timer + health regen
		}
		return;
	}

	UpdateStaminaAndMovement(DeltaSeconds);

	// PEEKING (local camera only): lean around the corner while RMB is held.
	if (IsLocallyControlled() && FirstPersonCamera)
	{
		float Target = (bSecondaryHeld && bAllowHoldBreath && !bIsDown && !bWantsToClimb) ? PeekInput : 0.f;

		// Each lean costs stamina: 5 still, 20 moving, 30 sprinting. No juice = no peek.
		if (FMath::Abs(Target) > 0.1f && !bPeekCostCharged)
		{
			const bool bMoving = GetVelocity().Size2D() > 40.f;
			const float Cost = (bMoving && bWantsToSprint) ? 30.f : bMoving ? 20.f : 5.f;
			if (GetStamina() >= Cost)
			{
				bPeekCostCharged = true;
				SpendStaminaLocal(Cost);
				if (!HasAuthority())
				{
					ServerSpendStamina(Cost);
				}
			}
			else
			{
				Target = 0.f; // too tired to lean
			}
		}
		else if (FMath::Abs(Target) <= 0.1f)
		{
			bPeekCostCharged = false;
		}

		CurrentPeek = FMath::FInterpTo(CurrentPeek, Target, DeltaSeconds, 8.f);
		if (FMath::Abs(CurrentPeek) > 0.01f || !FirstPersonCamera->GetRelativeLocation().Equals(CameraBaseRelLoc))
		{
			FirstPersonCamera->SetRelativeLocation(CameraBaseRelLoc + FVector(0.f, CurrentPeek * 60.f, FMath::Abs(CurrentPeek) * -6.f));
		}
	}
}

void ATdfCharacterBase::SpendStaminaLocal(float Amount)
{
	if (AttributeSet)
	{
		AttributeSet->SetStamina(FMath::Max(0.f, AttributeSet->GetStamina() - Amount));
	}
}

void ATdfCharacterBase::ServerSpendStamina_Implementation(float Amount)
{
	SpendStaminaLocal(FMath::Clamp(Amount, 0.f, 30.f));
}

UAbilitySystemComponent* ATdfCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATdfCharacterBase::SetBodyVisibleLocal(bool bVisible)
{
	if (BodyMesh)
	{
		BodyMesh->SetVisibility(bVisible);
	}
	if (GetMesh() && GetMesh()->GetSkeletalMeshAsset())
	{
		GetMesh()->SetVisibility(bVisible);
	}
}

float ATdfCharacterBase::GetHealth() const { return AttributeSet ? AttributeSet->GetHealth() : 0.f; }
float ATdfCharacterBase::GetMaxHealth() const { return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f; }
float ATdfCharacterBase::GetStamina() const { return AttributeSet ? AttributeSet->GetStamina() : 0.f; }
float ATdfCharacterBase::GetMaxStamina() const { return AttributeSet ? AttributeSet->GetMaxStamina() : 0.f; }

float ATdfCharacterBase::GetHealthBarValue() const { return GetHealth(); }
float ATdfCharacterBase::GetHealthBarMax() const { return GetMaxHealth(); }
FString ATdfCharacterBase::GetHealthBarLabel() const { return TEXT("Health"); }

void ATdfCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UTdfAttributeSet::GetMoveSpeedAttribute())
			.AddUObject(this, &ATdfCharacterBase::OnMoveSpeedChanged);

		InitializeAttributes();
		GrantDefaultAbilities();
	}
}

void ATdfCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client-side ASC init (PossessedBy only runs on the server).
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UTdfAttributeSet::GetMoveSpeedAttribute())
			.AddUObject(this, &ATdfCharacterBase::OnMoveSpeedChanged);
	}
}

void ATdfCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATdfCharacterBase, bIsDown);
	DOREPLIFETIME(ATdfCharacterBase, bIsStunned);
	DOREPLIFETIME(ATdfCharacterBase, bIsAsleep);
	DOREPLIFETIME(ATdfCharacterBase, bIsDead);
	DOREPLIFETIME(ATdfCharacterBase, bSeated);
}

void ATdfCharacterBase::InitializeAttributes()
{
}

void ATdfCharacterBase::GrantDefaultAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UTdfGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
		}
	}
}

void ATdfCharacterBase::OnAbility1Pressed()
{
	if (AbilitySystemComponent && DefaultAbilities.Num() > 0 && DefaultAbilities[0])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DefaultAbilities[0]);
	}
}

void ATdfCharacterBase::OnAbility2Pressed()
{
	if (AbilitySystemComponent && DefaultAbilities.Num() > 1 && DefaultAbilities[1])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DefaultAbilities[1]);
	}
}

void ATdfCharacterBase::OnAbility3Pressed()
{
	if (AbilitySystemComponent && DefaultAbilities.Num() > 2 && DefaultAbilities[2])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DefaultAbilities[2]);
	}
}

void ATdfCharacterBase::OnAbility4Pressed()
{
	if (AbilitySystemComponent && DefaultAbilities.Num() > 3 && DefaultAbilities[3])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DefaultAbilities[3]);
	}
}

void ATdfCharacterBase::OnAbility5Pressed()
{
	if (AbilitySystemComponent && DefaultAbilities.Num() > 4 && DefaultAbilities[4])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DefaultAbilities[4]);
	}
}

void ATdfCharacterBase::OnSecondaryAction()
{
	if (AbilitySystemComponent && DefaultAbilities.Num() > 5 && DefaultAbilities[5])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DefaultAbilities[5]);
	}
}

void ATdfCharacterBase::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	MoveSpeed = Data.NewValue;
}

void ATdfCharacterBase::OnHealthDepleted()
{
	// Killed while asleep -> permanent death (no second sleep roll).
	if (bIsAsleep)
	{
		bIsAsleep = false;
		if (GEngine && IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Killed in your sleep."));
		}
		Die();
		return;
	}

	if (bIsDown)
	{
		return;
	}
	bIsDown = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
}

void ATdfCharacterBase::OnKealthDepleted()
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
}

void ATdfCharacterBase::Recover()
{
	if (bIsDead)
	{
		return; // the dead don't get back up
	}
	bIsDown = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
	}
}

void ATdfCharacterBase::Die()
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;
	bIsDown = true;
	bIsAsleep = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("YOU DIED"));
	}

	if (HasAuthority())
	{
		// After a beat on the death screen, the player becomes a free-flying spectator.
		// The corpse stays in the world (win logic counts it; killers get their trophy).
		if (Cast<APlayerController>(GetController()))
		{
			FTimerHandle SpectateHandle;
			GetWorldTimerManager().SetTimer(SpectateHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (APlayerController* PC = Cast<APlayerController>(GetController()))
				{
					if (PC->PlayerState)
					{
						PC->PlayerState->SetIsSpectator(true);
					}
					PC->ChangeState(NAME_Spectating);
					PC->ClientGotoState(NAME_Spectating);
				}
			}), 3.f, false);
		}

		// Deaths can end the match.
		if (ATdfGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATdfGameMode>() : nullptr)
		{
			GameMode->CheckWinConditions();
		}
	}
}

ETdfDifficulty ATdfCharacterBase::GetMatchDifficulty() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const ATdfGameMode* GameMode = World->GetAuthGameMode<ATdfGameMode>())
		{
			return GameMode->Difficulty;
		}
	}
	return ETdfDifficulty::Normal;
}

void ATdfCharacterBase::ApplyStun(float Duration)
{
	bIsStunned = true;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ATdfCharacterBase::RemoveStun, Duration, false);
}

void ATdfCharacterBase::RemoveStun()
{
	bIsStunned = false;
	if (!bIsDown)
	{
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}
}

void ATdfCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATdfCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATdfCharacterBase::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATdfCharacterBase::OnJumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ATdfCharacterBase::OnJumpReleased);
	PlayerInputComponent->BindAction("Secondary", IE_Pressed, this, &ATdfCharacterBase::OnSecondaryPressedInternal);
	PlayerInputComponent->BindAction("Secondary", IE_Released, this, &ATdfCharacterBase::OnSecondaryReleasedInternal);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATdfCharacterBase::OnSprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATdfCharacterBase::OnSprintReleased);

	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &ATdfCharacterBase::OnWalkPressed);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &ATdfCharacterBase::OnWalkReleased);

	// E = hold breath for survivors (killers: signature ability).
	PlayerInputComponent->BindAction("HoldBreath", IE_Pressed, this, &ATdfCharacterBase::OnHoldBreathPressed);
	PlayerInputComponent->BindAction("HoldBreath", IE_Released, this, &ATdfCharacterBase::OnHoldBreathReleased);

	// Q needs press AND release (Dheer's watch is a hold).
	PlayerInputComponent->BindAction("Ability2", IE_Released, this, &ATdfCharacterBase::OnAbility2Released);

	// F = interact: hold to repair generators, press near a downed teammate to revive.
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ATdfCharacterBase::OnInteractPressed);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ATdfCharacterBase::OnInteractReleased);

	// Ability activation: Q / LMB / R / T. E is Hold Breath, C is vehicle.
	PlayerInputComponent->BindAction("Ability2", IE_Pressed, this, &ATdfCharacterBase::OnAbility2Pressed);
	PlayerInputComponent->BindAction("Ability3", IE_Pressed, this, &ATdfCharacterBase::OnAbility3Pressed);
	PlayerInputComponent->BindAction("Ability4", IE_Pressed, this, &ATdfCharacterBase::OnAbility4Pressed);
	PlayerInputComponent->BindAction("Ability5", IE_Pressed, this, &ATdfCharacterBase::OnAbility5Pressed);
	PlayerInputComponent->BindAction("Vehicle", IE_Pressed, this, &ATdfCharacterBase::ToggleVehicle);
}

void ATdfCharacterBase::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATdfCharacterBase::MoveRight(float Value)
{
	// PEEKING: while RMB is held, A/D lean the camera instead of strafing (survivors only).
	if (bSecondaryHeld && bAllowHoldBreath && !bIsDown)
	{
		PeekInput = FMath::Clamp(Value, -1.f, 1.f);
		return;
	}
	PeekInput = 0.f;

	if (Controller && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ATdfCharacterBase::OnSecondaryPressedInternal()
{
	bSecondaryHeld = true;
	OnSecondaryAction();
}

void ATdfCharacterBase::OnSecondaryReleasedInternal()
{
	bSecondaryHeld = false;
	PeekInput = 0.f;
	OnSecondaryReleased();
}

void ATdfCharacterBase::OnSprintPressed() { bWantsToSprint = true; if (!HasAuthority()) { ServerSetSprint(true); } }
void ATdfCharacterBase::OnSprintReleased() { bWantsToSprint = false; if (!HasAuthority()) { ServerSetSprint(false); } }
void ATdfCharacterBase::OnWalkPressed() { bWantsToWalk = true; if (!HasAuthority()) { ServerSetWalk(true); } }
void ATdfCharacterBase::OnWalkReleased() { bWantsToWalk = false; if (!HasAuthority()) { ServerSetWalk(false); } }

void ATdfCharacterBase::OnHoldBreathPressed()
{
	if (bAllowHoldBreath)
	{
		bHoldingBreath = true;
		if (!HasAuthority()) { ServerSetHoldBreath(true); }
	}
	else
	{
		// Killers don't breathe (much) — E is their signature ability instead.
		if (DefaultAbilities.Num() > 0 && DefaultAbilities[0])
		{
			OnAbility1Pressed();
		}
		else
		{
			OnSecondaryAction(); // no E-slot ability -> E mirrors right-click
		}
	}
}

void ATdfCharacterBase::OnHoldBreathReleased()
{
	bHoldingBreath = false;
	BreathZeroTime = 0.f;
	NextBreathDamageAt = 2.f;
	if (!HasAuthority()) { ServerSetHoldBreath(false); }
}

void ATdfCharacterBase::ServerSetSprint_Implementation(bool bNewValue) { bWantsToSprint = bNewValue; }
void ATdfCharacterBase::ServerSetWalk_Implementation(bool bNewValue) { bWantsToWalk = bNewValue; }
void ATdfCharacterBase::ServerSetHoldBreath_Implementation(bool bNewValue) { if (bAllowHoldBreath) { bHoldingBreath = bNewValue; } }

void ATdfCharacterBase::ServerSetInteract_Implementation(bool bNewValue)
{
	bWantsToInteract = bNewValue;
	if (bNewValue)
	{
		HandleServerInteract();
	}
	else
	{
		CancelRevive();
	}
}

void ATdfCharacterBase::OnInteractPressed()
{
	bWantsToInteract = true;
	if (HasAuthority())
	{
		HandleServerInteract();
	}
	else
	{
		ServerSetInteract(true);
	}
}

AActor* ATdfCharacterBase::GetLookAtActor(float Range) const
{
	if (!GetWorld())
	{
		return nullptr;
	}
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const FVector Start = GetPawnViewLocation();
	const FVector End = Start + GetControlRotation().Vector() * Range;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.GetActor();
	}
	return nullptr;
}

void ATdfCharacterBase::SitOn(ATdfSeat* Seat)
{
	if (!HasAuthority() || !Seat || Seat->Occupant.IsValid() || bSeated || bIsDown || bIsStunned)
	{
		return;
	}
	bSeated = true;
	SeatedOn = Seat;
	Seat->Occupant = this;
	SetActorLocation(Seat->GetActorLocation() + FVector(0.f, 0.f, 110.f));
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
			Seat->bHideSpot ? TEXT("Hidden. Don't. Move.") : TEXT("Seated (E to stand up)"));
	}
	if (ATdfGameMode* GameMode = GetWorld()->GetAuthGameMode<ATdfGameMode>())
	{
		GameMode->RecheckMonitorSeating();
	}
}

void ATdfCharacterBase::StandUp()
{
	if (!HasAuthority() || !bSeated)
	{
		return;
	}
	bSeated = false;
	if (ATdfSeat* Seat = SeatedOn.Get())
	{
		Seat->Occupant = nullptr;
		SetActorLocation(Seat->GetActorLocation() + Seat->GetActorForwardVector() * 90.f + FVector(0.f, 0.f, 110.f));
	}
	SeatedOn = nullptr;
	if (!bIsDown && !bIsStunned)
	{
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}
	if (ATdfGameMode* GameMode = GetWorld()->GetAuthGameMode<ATdfGameMode>())
	{
		GameMode->RecheckMonitorSeating();
	}
}

void ATdfCharacterBase::HandleServerInteract()
{
	// DJ has your leg: every E press is a 1% chance to rip free. Nothing else works.
	if (ATdfRunnerCharacter* DraggedSelf = Cast<ATdfRunnerCharacter>(this))
	{
		if (DraggedSelf->bDraggedByDJ)
		{
			for (TActorIterator<ATdfDJDog> It(GetWorld()); It; ++It)
			{
				if ((*It)->TryStruggleFree(DraggedSelf))
				{
					break;
				}
			}
			return;
		}
	}

	// Tung Tung's E is for kidnapping, not first aid.
	if (ATdfKiller_TungTung* TungTung = Cast<ATdfKiller_TungTung>(this))
	{
		TungTung->TryCarryOrDeliver();
		return;
	}
	// Lucki's E is car business: sit in / handle the keys.
	if (ATdfKiller_Lucki* Lucki = Cast<ATdfKiller_Lucki>(this))
	{
		Lucki->HandleCarInteract();
		return;
	}

	// Already sitting or hiding? E gets you up.
	if (bSeated)
	{
		StandUp();
		return;
	}

	// Whatever you're looking at: seats, wardrobes, routers... or an unlocked Prius.
	if (AActor* Target = GetLookAtActor(420.f))
	{
		if (ATdfSeat* Seat = Cast<ATdfSeat>(Target))
		{
			SitOn(Seat);
			return;
		}
		if (ATdfRouter* Router = Cast<ATdfRouter>(Target))
		{
			Router->CheckRouter(this);
			return;
		}
		// Lucki left it unlocked with the keys in? A runner can STEAL them.
		if (ATdfPriusProp* Prius = Cast<ATdfPriusProp>(Target))
		{
			if (ATdfRunnerCharacter* Thief = Cast<ATdfRunnerCharacter>(this))
			{
				Prius->TryStealKeys(Thief);
				return;
			}
		}
	}

	TryReviveNearby();
}

void ATdfCharacterBase::OnInteractReleased()
{
	bWantsToInteract = false;
	if (HasAuthority())
	{
		CancelRevive();
	}
	else
	{
		ServerSetInteract(false);
	}
}

void ATdfCharacterBase::TryReviveNearby()
{
	if (!HasAuthority() || bIsDown || bIsDead || !GetWorld())
	{
		return;
	}

	// Only runners revive teammates — the killer's F key is not a medical license.
	const ATdfRunnerCharacter* SelfRunner = Cast<ATdfRunnerCharacter>(this);
	if (!SelfRunner)
	{
		return;
	}

	// "LOCK IN!" — snap a doomscrolling Musa out of his reels.
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Other = *It;
		if (Other && Other != this && Other->bIsStunned && !Other->bIsDown && !Other->IsDead()
			&& Other->RunnerRole == ETdfRunnerRole::Trickster
			&& FVector::Dist(Other->GetActorLocation(), GetActorLocation()) <= ReviveRange)
		{
			Other->RemoveStun();
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("\"LOCK IN, MUSA!\" - he locks in"));
			}
			return;
		}
	}

	for (TActorIterator<ATdfCharacterBase> It(GetWorld()); It; ++It)
	{
		ATdfCharacterBase* Other = *It;
		if (!Other || Other == this || !Other->bIsDown || Other->bIsDead)
		{
			continue;
		}
		if (const ATdfRunnerCharacter* OtherRunner = Cast<ATdfRunnerCharacter>(Other))
		{
			// Escaped runners are hidden at the exit; don't "revive" them.
			if (OtherRunner->HasEscaped())
			{
				continue;
			}
		}
		if (FVector::Dist(Other->GetActorLocation(), GetActorLocation()) <= ReviveRange)
		{
			ReviveTarget = Other;
			GetWorldTimerManager().SetTimer(ReviveTimerHandle, this, &ATdfCharacterBase::FinishRevive, ReviveDuration, false);
			if (GEngine && IsLocallyControlled())
			{
				GEngine->AddOnScreenDebugMessage(-1, ReviveDuration, FColor::Green, TEXT("Reviving teammate... stay close"));
			}
			return;
		}
	}
}

void ATdfCharacterBase::FinishRevive()
{
	ATdfCharacterBase* Target = ReviveTarget.Get();
	ReviveTarget = nullptr;

	if (!Target || !Target->bIsDown || Target->bIsDead || bIsDown || bIsDead)
	{
		return;
	}
	if (FVector::Dist(Target->GetActorLocation(), GetActorLocation()) > ReviveRange * 1.4f)
	{
		return; // wandered off mid-revive
	}

	Target->Recover();
	if (Target->AttributeSet)
	{
		Target->AttributeSet->SetHealth(Target->AttributeSet->GetMaxHealth() * 0.5f);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Teammate revived!"));
	}
}

void ATdfCharacterBase::CancelRevive()
{
	GetWorldTimerManager().ClearTimer(ReviveTimerHandle);
	ReviveTarget = nullptr;
}

bool ATdfCharacterBase::IsFacingClimbableTree() const
{
	if (!GetWorld())
	{
		return false;
	}
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * 190.f;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.GetComponent() && Hit.GetComponent()->GetFName() == FName(TEXT("TreeISM"));
	}
	return false;
}

void ATdfCharacterBase::ServerSetClimb_Implementation(bool bNewValue)
{
	bWantsToClimb = bNewValue;
}

void ATdfCharacterBase::OnJumpPressed()
{
	if (bIsDown || bIsStunned)
	{
		return;
	}

	// Against a trunk? Hold Jump to climb instead (Troos is too fat — see runners.md).
	if (CanClimbTrees() && IsFacingClimbableTree())
	{
		bWantsToClimb = true;
		if (!HasAuthority()) { ServerSetClimb(true); }
		return;
	}

	if (!AttributeSet)
	{
		Jump();
		return;
	}

	const float Cost = AttributeSet->GetMaxStamina() * JumpStaminaCostFraction;
	if (AttributeSet->GetStamina() >= Cost)
	{
		AttributeSet->SetStamina(AttributeSet->GetStamina() - Cost);
		Jump();
	}
	else if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Too tired to jump"));
	}
}

void ATdfCharacterBase::OnJumpReleased()
{
	StopJumping();
	if (bWantsToClimb)
	{
		bWantsToClimb = false;
		if (!HasAuthority()) { ServerSetClimb(false); }
	}
}

float ATdfCharacterBase::StaminaRateCurve(float Fraction) const
{
	Fraction = FMath::Clamp(Fraction, 0.f, 1.f);
	const float Bell = FMath::Sin(PI * Fraction);           // slow at both ends, fast in the middle
	const float TopDamp = 1.f - FMath::Pow(Fraction, 3.f);  // extra slow approaching full
	float Rate = FMath::Lerp(CurveFloor, 1.f, Bell * TopDamp);

	// Last 5% before full crawls to ~10% rate (super slow to top off / drain near full).
	if (Fraction > 0.95f)
	{
		const float T = (Fraction - 0.95f) / 0.05f; // 0..1 across the final 5%
		Rate *= FMath::Lerp(1.f, 0.1f, T);
	}
	return Rate;
}

void ATdfCharacterBase::ApplyBreathDamage()
{
	if (!AttributeSet)
	{
		return;
	}

	const float NewHealth = FMath::Max(0.f, AttributeSet->GetHealth() - BreathDamagePerTick);
	AttributeSet->SetHealth(NewHealth);

	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, FString::Printf(TEXT("Suffocating! Health %.0f"), NewHealth));
	}

	if (NewHealth <= 0.f)
	{
		if (!bHasBreathDiedBefore)
		{
			bHasBreathDiedBefore = true;
			if (FMath::FRand() < BreathDeathChance)
			{
				Die();        // 80% — suffocation kills you outright
			}
			else
			{
				EnterSleep(); // 20% — you pass out for 30s instead
			}
		}
		else
		{
			Die(); // the sleep save only happens once
		}
	}
}

void ATdfCharacterBase::EnterSleep()
{
	bIsAsleep = true;
	bIsDown = true;
	bHoldingBreath = false;
	bWantsToSprint = false;
	SleepTimeRemaining = SleepDuration;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	if (AttributeSet)
	{
		AttributeSet->SetHealth(1.f); // unconscious, not dead; regenerates while asleep
	}

	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("You passed out! Waking in 30s (you can still be killed)"));
	}
}

void ATdfCharacterBase::WakeUp()
{
	bIsAsleep = false;
	bIsDown = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
	}

	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("You woke up!"));
	}
}

void ATdfCharacterBase::UpdateSleep(float DeltaSeconds)
{
	if (AttributeSet)
	{
		AttributeSet->SetHealth(FMath::Min(AttributeSet->GetMaxHealth(),
			AttributeSet->GetHealth() + SleepHealthRegenPerSecond * DeltaSeconds));
	}

	SleepTimeRemaining -= DeltaSeconds;

	if (SleepTimeRemaining <= 0.f)
	{
		WakeUp();
	}
}

void ATdfCharacterBase::UpdateStaminaAndMovement(float DeltaSeconds)
{
	if (!AttributeSet || bIsDown)
	{
		return; // downed pawns skip the stamina sim
	}

	// Only the server (authority) and the owning client run the sim; sim-proxies just replicate.
	if (!HasAuthority() && !IsLocallyControlled())
	{
		return;
	}

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move)
	{
		return;
	}

	const float MaxStamina = AttributeSet->GetMaxStamina();
	float Stamina = AttributeSet->GetStamina();
	const float Fraction = (MaxStamina > 0.f) ? (Stamina / MaxStamina) : 0.f;
	const bool bBreathing = bHoldingBreath && bAllowHoldBreath;

	// --- Tree climbing: hold Jump against a trunk to shimmy up; add Ctrl to hang on ---
	if (bWantsToClimb)
	{
		if (CanClimbTrees() && Stamina > 0.f && IsFacingClimbableTree() && !bIsStunned)
		{
			// Freeze the body's facing so looking around doesn't twist you off the trunk.
			if (!bClimbLookDecoupled)
			{
				bClimbLookDecoupled = true;
				bUseControllerRotationYaw = false;
				if (APlayerController* PC = Cast<APlayerController>(GetController()))
				{
					if (PC->PlayerCameraManager)
					{
						const float AnchorYaw = GetActorRotation().Yaw;
						PC->PlayerCameraManager->ViewYawMin = AnchorYaw - 135.f;
						PC->PlayerCameraManager->ViewYawMax = AnchorYaw + 135.f;
					}
				}
			}

			Move->SetMovementMode(MOVE_Flying);
			const bool bHanging = bWantsToWalk; // Ctrl: cling to the trunk + look around freely
			Move->Velocity = bHanging ? FVector::ZeroVector : FVector(0.f, 0.f, ClimbSpeed);
			const float Cost = bHanging ? HangStaminaPerSecond : ClimbStaminaPerSecond;
			AttributeSet->SetStamina(FMath::Max(0.f, Stamina - Cost * DeltaSeconds));
			LastSpeedMultiplier = 0.f;
			return;
		}
		bWantsToClimb = false;
	}
	if (bClimbLookDecoupled)
	{
		// Off the trunk: re-couple facing to the camera and unclamp the view.
		bClimbLookDecoupled = false;
		bUseControllerRotationYaw = true;
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->ViewYawMin = 0.f;
				PC->PlayerCameraManager->ViewYawMax = 359.998993f;
			}
		}
	}
	if (Move->MovementMode == MOVE_Flying)
	{
		Move->SetMovementMode(MOVE_Falling); // let go / trunk ended -> gravity wins
	}

	const FVector Velocity = GetVelocity();
	const float HorizontalSpeed = Velocity.Size2D();
	const bool bMoving = HorizontalSpeed > 10.f;

	// Uphill amount (0..1) from upward motion while grounded.
	float Uphill = 0.f;
	if (Move->IsMovingOnGround() && bMoving)
	{
		Uphill = FMath::Clamp(Velocity.Z / FMath::Max(HorizontalSpeed, 1.f), 0.f, 1.f);
	}

	float SpeedMultiplier = 1.f;

	// Sprint is allowed while holding breath (capped + burns much faster).
	const bool bSprinting = bWantsToSprint && CanSprint() && bMoving && Stamina > 0.f
		&& !bSprintExhausted && !bIsStunned;

	if (bSprinting)
	{
		float DrainMult = 1.f;

		if (bBreathing)
		{
			// Holding breath: capped to 1.5x, drains 5x faster, no full lock.
			SpeedMultiplier = SprintPartialMultiplier;
			bFullSprintLock = false;
			DrainMult = HoldBreathSprintDrainMultiplier;
		}
		else
		{
			// On sprint start, lock full 3x only if begun from a full bar.
			if (!bWasSprinting)
			{
				bFullSprintLock = (Fraction >= FullStartThreshold);

				// SUPER RUN: spend a banked charge — this whole run drains at half rate.
				if (SuperRunCharges > 0)
				{
					--SuperRunCharges;
					bSuperRunActive = true;
					if (GEngine && IsLocallyControlled())
					{
						GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("SUPER RUN!"));
					}
				}
			}
			// Full-bar start => locked 3x. Otherwise a gradient that tops out at 2.5x.
			SpeedMultiplier = bFullSprintLock
				? SprintFullMultiplier
				: FMath::Lerp(SprintPartialMultiplier, SprintGradientMax, Fraction);
		}

		if (bSuperRunActive)
		{
			DrainMult *= 0.5f; // the bar lasts twice as long
		}

		const float Drain = SprintDrainPerSecond * StaminaRateCurve(Fraction)
			* (1.f + Uphill * UphillDrainBonus) * DrainMult * DeltaSeconds;
		Stamina = FMath::Max(0.f, Stamina - Drain);
		AttributeSet->SetStamina(Stamina);

		if (Stamina <= 0.f)
		{
			bSprintExhausted = true; // winded
			bFullSprintLock = false;
		}
	}
	else
	{
		bFullSprintLock = false;

		// Non-sprint base speed: winded penalty and/or deliberate slow-walk (most restrictive wins).
		if (bSprintExhausted)
		{
			SpeedMultiplier = FMath::Min(SpeedMultiplier, WindedWalkMultiplier);
		}
		if (bWantsToWalk)
		{
			SpeedMultiplier = FMath::Min(SpeedMultiplier, WalkSpeedMultiplier);
		}

		if (bBreathing)
		{
			// Slow breath drain, no regen.
			Stamina = FMath::Max(0.f, Stamina - HoldBreathDrainPerSecond * DeltaSeconds);
			AttributeSet->SetStamina(Stamina);
		}
		else if (Stamina < MaxStamina)
		{
			Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenPerSecond * StaminaRateCurve(Fraction) * DeltaSeconds);
			AttributeSet->SetStamina(Stamina);
		}

		// Lift "winded" once recovered to the threshold (only relevant if winded).
		if (bSprintExhausted && Stamina >= SprintRecoverFraction * MaxStamina)
		{
			bSprintExhausted = false;
		}
	}

	// The super run ends with the run itself.
	if (!bSprinting && bSuperRunActive)
	{
		bSuperRunActive = false;
	}

	// Bank a SUPER RUN for every full minute spent with a topped-off bar.
	if (!bSprinting && Stamina >= MaxStamina * 0.999f)
	{
		FullStaminaTime += DeltaSeconds;
		if (FullStaminaTime >= SuperRunEarnSeconds)
		{
			FullStaminaTime = 0.f;
			if (SuperRunCharges < SuperRunMaxCharges)
			{
				++SuperRunCharges;
				if (GEngine && IsLocallyControlled())
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
						FString::Printf(TEXT("SUPER RUN ready (x%d) - next sprint lasts twice as long"), SuperRunCharges));
				}
			}
		}
	}
	else if (Stamina < MaxStamina * 0.999f)
	{
		FullStaminaTime = 0.f;
	}

	// Suffocation is tracked independently of sprinting; damage/death applied on the server only.
	if (bBreathing && Stamina <= 0.f)
	{
		BreathZeroTime += DeltaSeconds;
		if (BreathZeroTime >= NextBreathDamageAt)
		{
			if (HasAuthority())
			{
				ApplyBreathDamage();
			}
			NextBreathDamageAt += 2.f;
		}
	}
	else
	{
		BreathZeroTime = 0.f;
		NextBreathDamageAt = 2.f;
	}

	bWasSprinting = bSprinting;

	const float UphillSpeedFactor = 1.f - (Uphill * UphillSpeedPenalty);
	LastSpeedMultiplier = SpeedMultiplier * UphillSpeedFactor;
	Move->MaxWalkSpeed = MoveSpeed * SpeedToCmPerSec * LastSpeedMultiplier;
}

void ATdfCharacterBase::ApplyMoveSpeed(float NewDesignSpeed)
{
	MoveSpeed = NewDesignSpeed;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = NewDesignSpeed * SpeedToCmPerSec;
	}
}

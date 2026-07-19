#include "TdfDeployables.h"
#include "TdfAttributeSet.h"
#include "TdfRunnerCharacter.h"
#include "TdfKillerCharacter.h"
#include "TdfKillers.h"
#include "TdfTypes.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

// --- Decoy ---

ATdfDecoy::ATdfDecoy()
{
	bReplicates = true;
	InitialLifeSpan = 30.f;

	DecoyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DecoyMesh"));
	SetRootComponent(DecoyMesh);
	DecoyMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.76f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (MeshAsset.Succeeded())
	{
		DecoyMesh->SetStaticMesh(MeshAsset.Object);
	}
}

// --- The parked Prius ---

ATdfPriusProp::ATdfPriusProp()
{
	// MUST be set here — flipping it in BeginPlay is too late and the engine
	// never cranks, which was the "stuck on cranking 5 seconds" bug.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarMesh"));
	SetRootComponent(CarMesh);
	CarMesh->SetRelativeScale3D(FVector(4.2f, 1.9f, 1.2f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		CarMesh->SetStaticMesh(MeshAsset.Object);
	}

	// Headlights (SPACE toggles them while driving).
	HeadlightL = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightL"));
	HeadlightL->SetupAttachment(CarMesh);
	HeadlightL->SetRelativeLocation(FVector(52.f, -18.f, 8.f));
	HeadlightL->SetIntensity(80000.f);
	HeadlightL->SetOuterConeAngle(28.f);
	HeadlightL->SetAttenuationRadius(4500.f);
	HeadlightL->SetVisibility(false);
	HeadlightR = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightR"));
	HeadlightR->SetupAttachment(CarMesh);
	HeadlightR->SetRelativeLocation(FVector(52.f, 18.f, 8.f));
	HeadlightR->SetIntensity(80000.f);
	HeadlightR->SetOuterConeAngle(28.f);
	HeadlightR->SetAttenuationRadius(4500.f);
	HeadlightR->SetVisibility(false);
}

void ATdfPriusProp::BeginPlay()
{
	Super::BeginPlay();

	// It has to be BLUE.
	if (UMaterialInterface* Base = Cast<UMaterialInterface>(
		StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))))
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.15f, 0.6f));
		CarMesh->SetMaterial(0, MID);
	}
}

void ATdfPriusProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfPriusProp, bLocked);
	DOREPLIFETIME(ATdfPriusProp, bKeysInserted);
	DOREPLIFETIME(ATdfPriusProp, bEngineOn);
	DOREPLIFETIME(ATdfPriusProp, EngineStartRemaining);
	DOREPLIFETIME(ATdfPriusProp, Petrol);
	DOREPLIFETIME(ATdfPriusProp, Battery);
	DOREPLIFETIME(ATdfPriusProp, bHeadlightsOn);
	DOREPLIFETIME(ATdfPriusProp, bKeysStolen);
}

void ATdfPriusProp::ToggleHeadlights()
{
	if (!HasAuthority())
	{
		return;
	}
	bHeadlightsOn = !bHeadlightsOn;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, bHeadlightsOn ? TEXT("Headlights ON") : TEXT("Headlights off"));
	}
}

void ATdfPriusProp::TryStealKeys(ATdfRunnerCharacter* Thief)
{
	if (!HasAuthority() || !Thief)
	{
		return;
	}
	if (bLocked)
	{
		return; // locked door beats sticky fingers
	}
	if (!bKeysInserted)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("No keys inside...")); }
		return;
	}
	if (Driver.IsValid())
	{
		return; // Lucki's literally sitting in it
	}
	bKeysInserted = false;
	bKeysStolen = true;
	KeyThief = Thief;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("SOMEONE STOLE THE PRIUS KEYS! It won't start until the thief is dealt with."));
	}
}

void ATdfPriusProp::ToggleLock()
{
	if (!HasAuthority())
	{
		return;
	}
	bLocked = !bLocked;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, bLocked ? TEXT("*clunk* Prius LOCKED") : TEXT("*beep beep* Prius unlocked"));
	}
}

void ATdfPriusProp::ToggleKeys()
{
	if (!HasAuthority())
	{
		return;
	}
	if (bEngineOn || EngineStartRemaining > 0.f)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("Kill the engine first (RMB)")); }
		return;
	}
	bKeysInserted = !bKeysInserted;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, bKeysInserted ? TEXT("Keys in the ignition") : TEXT("Keys pocketed"));
	}
}

void ATdfPriusProp::RequestEngineToggle()
{
	if (!HasAuthority())
	{
		return;
	}
	if (bEngineOn)
	{
		bEngineOn = false;
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Engine off.")); }
		return;
	}
	if (EngineStartRemaining > 0.f)
	{
		return; // already cranking
	}
	if (!bKeysInserted)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("No keys in the ignition (E)")); }
		return;
	}
	if (Petrol <= 0.f && Battery <= 0.f)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Completely dry. This Prius is furniture now.")); }
		return;
	}
	EngineStartRemaining = EngineStartSeconds;
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("*krkrkrkr* cranking the engine...")); }
}

void ATdfPriusProp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Headlight visuals run on every machine off the replicated flag.
	if (HeadlightL) { HeadlightL->SetVisibility(bHeadlightsOn && !IsHidden()); }
	if (HeadlightR) { HeadlightR->SetVisibility(bHeadlightsOn && !IsHidden()); }

	if (!HasAuthority())
	{
		return;
	}

	// Stolen keys find their way home when the thief dies (Lucki keeps a spare, allegedly).
	if (bKeysStolen && (!KeyThief.IsValid() || KeyThief->IsDead()))
	{
		bKeysStolen = false;
		bKeysInserted = true;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Cyan, TEXT("The Prius keys are back in the ignition."));
		}
	}

	// Cranking.
	if (EngineStartRemaining > 0.f)
	{
		EngineStartRemaining = FMath::Max(0.f, EngineStartRemaining - DeltaSeconds);
		if (EngineStartRemaining <= 0.f && bKeysInserted)
		{
			bEngineOn = true;
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("*vroom* (quietly, it's a Prius)")); }
		}
	}

	// Hybrid fuel model while being driven.
	if (bEngineOn && Driver.IsValid())
	{
		const float Speed2D = Driver->GetVelocity().Size2D();
		if (Speed2D > 600.f && Petrol > 0.f)
		{
			// Fast = petrol engine, trickle-charges the battery.
			Petrol = FMath::Max(0.f, Petrol - PetrolBurnPerSecond * DeltaSeconds);
			Battery = FMath::Min(100.f, Battery + 0.2f * DeltaSeconds);
			if (Petrol <= 0.f && GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("Petrol's gone - electric crawl only"));
			}
		}
		else if (Speed2D > 40.f)
		{
			// Slow (or out of petrol) = silent electric.
			Battery = FMath::Max(0.f, Battery - BatteryBurnPerSecond * DeltaSeconds);
		}
		if (Petrol <= 0.f && Battery <= 0.f)
		{
			bEngineOn = false;
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("The Prius has died.")); }
		}
	}

	// Refuelling: engine off, parked at a station.
	if (!bEngineOn && Petrol < 100.f)
	{
		for (TActorIterator<ATdfGasStation> It(GetWorld()); It; ++It)
		{
			const ATdfGasStation* Station = *It;
			if (Station && FVector::Dist(Station->GetActorLocation(), GetActorLocation()) <= Station->RefuelRadius)
			{
				Petrol = FMath::Min(100.f, Petrol + RefuelPerSecond * DeltaSeconds);
				break;
			}
		}
	}
}

// --- MOST HATED S.O (the gas station) ---

ATdfGasStation::ATdfGasStation()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	SetRootComponent(PadMesh);
	PadMesh->SetRelativeScale3D(FVector(14.f, 9.f, 0.2f));

	auto MakePart = [this](const TCHAR* Name, const FVector& RelLoc, const FVector& Scale) {
		UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Comp->SetupAttachment(PadMesh);
		// Parent pad is scaled — compensate in relative space.
		Comp->SetRelativeLocation(RelLoc);
		Comp->SetRelativeScale3D(Scale);
		return Comp;
	};

	// Relative to the pad's (14, 9, 0.2) scale.
	PillarA = MakePart(TEXT("PillarA"), FVector(-30.f, 0.f, 1250.f), FVector(0.03f, 0.05f, 25.f));
	PillarB = MakePart(TEXT("PillarB"), FVector(30.f, 0.f, 1250.f), FVector(0.03f, 0.05f, 25.f));
	CanopyMesh = MakePart(TEXT("CanopyMesh"), FVector(0.f, 0.f, 2600.f), FVector(1.02f, 1.05f, 1.5f));
	PumpA = MakePart(TEXT("PumpA"), FVector(-15.f, 0.f, 400.f), FVector(0.045f, 0.08f, 7.f));
	PumpB = MakePart(TEXT("PumpB"), FVector(15.f, 0.f, 400.f), FVector(0.045f, 0.08f, 7.f));

	if (CubeAsset.Succeeded())
	{
		PadMesh->SetStaticMesh(CubeAsset.Object);
		PillarA->SetStaticMesh(CubeAsset.Object);
		PillarB->SetStaticMesh(CubeAsset.Object);
		CanopyMesh->SetStaticMesh(CubeAsset.Object);
		PumpA->SetStaticMesh(CubeAsset.Object);
		PumpB->SetStaticMesh(CubeAsset.Object);
	}

	SignText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SignText"));
	SignText->SetupAttachment(CanopyMesh);
	SignText->SetRelativeLocation(FVector(0.f, -55.f, 40.f));
	SignText->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	SignText->SetRelativeScale3D(FVector(1.f, 1.f / 14.f * 6.f, 1.f / 1.5f * 4.f));
	SignText->SetText(FText::FromString(TEXT("MOST HATED S.O")));
	SignText->SetTextRenderColor(FColor::Red);
	SignText->SetHorizontalAlignment(EHTA_Center);
}

void ATdfGasStation::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Base = Cast<UMaterialInterface>(
		StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))))
	{
		auto Tint = [&](UStaticMeshComponent* Comp, const FLinearColor& Color) {
			if (Comp)
			{
				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
				MID->SetVectorParameterValue(TEXT("Color"), Color);
				Comp->SetMaterial(0, MID);
			}
		};
		Tint(PadMesh, FLinearColor(0.15f, 0.15f, 0.17f));
		Tint(CanopyMesh, FLinearColor(0.75f, 0.08f, 0.08f));
		Tint(PillarA, FLinearColor(0.8f, 0.8f, 0.8f));
		Tint(PillarB, FLinearColor(0.8f, 0.8f, 0.8f));
		Tint(PumpA, FLinearColor(0.9f, 0.9f, 0.95f));
		Tint(PumpB, FLinearColor(0.9f, 0.9f, 0.95f));
	}
}

// --- Musa's hidden speaker ---

ATdfSpeaker::ATdfSpeaker()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Survivors see the little speaker (so they don't waste hides near it) — the killer never does.
	SpeakerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpeakerMesh"));
	SetRootComponent(SpeakerMesh);
	SpeakerMesh->SetRelativeScale3D(FVector(0.28f, 0.2f, 0.35f));
	SpeakerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		SpeakerMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ATdfSpeaker::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInterface* Base = Cast<UMaterialInterface>(
		StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))))
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.1f, 0.8f, 0.9f));
		SpeakerMesh->SetMaterial(0, MID);
	}
}

void ATdfSpeaker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Per-viewer visibility (purely local): killers never see it, survivors do.
	const APlayerController* LocalPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	SetActorHiddenInGame(LocalPC && Cast<ATdfKillerCharacter>(LocalPC->GetPawn()) != nullptr);

	if (!HasAuthority())
	{
		return;
	}

	CooldownRemaining = FMath::Max(0.f, CooldownRemaining - DeltaSeconds);
	if (CooldownRemaining > 0.f)
	{
		return;
	}

	for (TActorIterator<ATdfKillerCharacter> It(GetWorld()); It; ++It)
	{
		const ATdfKillerCharacter* Killer = *It;
		if (Killer && !Killer->IsDead()
			&& FVector::Dist(Killer->GetActorLocation(), GetActorLocation()) <= TriggerRadius)
		{
			CooldownRemaining = RetriggerCooldown;
			Multicast_PlayVoices(GetActorLocation());
			break;
		}
	}
}

void ATdfSpeaker::Multicast_PlayVoices_Implementation(FVector Location)
{
	// TODO(audio): play back proximity-chat recorded around Musa. Needs VOIP capture + a sound asset.
	// For now the killer gets a very convincing noise cue at the speaker's spot.
	const APlayerController* LocalPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (LocalPC && Cast<ATdfKillerCharacter>(LocalPC->GetPawn()))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White, TEXT("...voices. You hear people whispering nearby."));
		}
		DrawDebugSphere(GetWorld(), Location + FVector(0, 0, 100.f), 100.f, 10, FColor::White, false, 3.f);
	}
}

// --- Food pickup ---

ATdfFoodPickup::ATdfFoodPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	FoodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoodMesh"));
	SetRootComponent(FoodMesh);
	FoodMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.3f));
	FoodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		FoodMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ATdfFoodPickup::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInterface* Base = Cast<UMaterialInterface>(
		StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))))
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.9f, 0.7f, 0.2f));
		FoodMesh->SetMaterial(0, MID);
	}
}

void ATdfFoodPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority())
	{
		return;
	}

	// Hungry runners heal; a peckish Lucki refuels.
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (Runner && !Runner->IsDead() && !Runner->bIsDown
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= PickupRadius)
		{
			if (UTdfAttributeSet* Attr = Runner->GetTdfAttributeSet())
			{
				if (Attr->GetHealth() < Attr->GetMaxHealth())
				{
					Attr->SetHealth(FMath::Min(Attr->GetMaxHealth(), Attr->GetHealth() + 25.f));
					if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("*nom* +25 health")); }
					Destroy();
					return;
				}
			}
		}
	}
	for (TActorIterator<ATdfKiller_Lucki> It(GetWorld()); It; ++It)
	{
		ATdfKiller_Lucki* Lucki = *It;
		if (Lucki && !Lucki->IsDead() && Lucki->Hunger < 95.f
			&& FVector::Dist(Lucki->GetActorLocation(), GetActorLocation()) <= PickupRadius)
		{
			Lucki->Hunger = FMath::Min(100.f, Lucki->Hunger + 40.f);
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("*NOM* Lucki found snacks")); }
			Destroy();
			return;
		}
	}
}

// --- Repair kit ---

ATdfRepairKit::ATdfRepairKit()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	KitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KitMesh"));
	SetRootComponent(KitMesh);
	KitMesh->SetRelativeScale3D(FVector(0.5f, 0.35f, 0.3f));
	KitMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		KitMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ATdfRepairKit::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInterface* Base = Cast<UMaterialInterface>(
		StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"))))
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.95f, 0.45f, 0.05f));
		KitMesh->SetMaterial(0, MID);
	}
}

void ATdfRepairKit::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority())
	{
		return;
	}
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (Runner && !Runner->IsDead() && !Runner->bIsDown && !Runner->bHasRepairKit
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= PickupRadius)
		{
			Runner->bHasRepairKit = true;
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Repair kit picked up")); }
			Destroy();
			return;
		}
	}
}

// --- Weapon pickup ---

ATdfWeaponPickup::ATdfWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetRelativeScale3D(FVector(1.1f, 0.25f, 0.25f));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		WeaponMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ATdfWeaponPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (Runner && !Runner->IsDead() && !Runner->bIsDown
			&& Runner->CurrentWeapon == ETdfWeaponType::None
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= PickupRadius)
		{
			Runner->CurrentWeapon = WeaponType;
			switch (WeaponType)
			{
			case ETdfWeaponType::Axon:       Runner->WeaponDurability = 8; break; // high durability
			case ETdfWeaponType::TripleTBat: Runner->WeaponDurability = 2; break; // two hits
			case ETdfWeaponType::Taser:      Runner->WeaponDurability = 1; break; // one zap
			default:                         Runner->WeaponDurability = 0; break;
			}
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Weapon picked up"));
			}
			Destroy();
			return;
		}
	}
}

// --- Finder device (hardcore generator gadget) ---

ATdfFinderDevice::ATdfFinderDevice()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
	SetRootComponent(DeviceMesh);
	DeviceMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.15f));
	DeviceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		DeviceMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ATdfFinderDevice::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (Runner && !Runner->IsDead() && !Runner->bIsDown && !Runner->bHasGeneratorFinder
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= PickupRadius)
		{
			Runner->bHasGeneratorFinder = true;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Generator finder picked up!"));
			}
			Destroy();
			return;
		}
	}
}

// --- Nani trap ---

ATdfNaniTrap::ATdfNaniTrap()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	SetRootComponent(TrapMesh);
	TrapMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));
	TrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (MeshAsset.Succeeded())
	{
		TrapMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ATdfNaniTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	CooldownRemaining = FMath::Max(0.f, CooldownRemaining - DeltaSeconds);
	if (CooldownRemaining > 0.f)
	{
		return;
	}

	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		const ATdfRunnerCharacter* Runner = *It;
		if (Runner && !Runner->IsDead() && !Runner->HasEscaped()
			&& Runner->RunnerRole != ETdfRunnerRole::Tank            // Troos is immune to Lucki's traps
			&& Runner->GetVelocity().Size2D() > 10.f
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= TriggerRadius)
		{
			CooldownRemaining = RetriggerCooldown;
			Multicast_Alert(Runner->GetActorLocation());
			break;
		}
	}
}

void ATdfNaniTrap::Multicast_Alert_Implementation(FVector Location)
{
	// Only killer players get Nani's tip-off.
	const APlayerController* LocalPC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (LocalPC && Cast<ATdfKillerCharacter>(LocalPC->GetPawn()))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("NANI SPOTTED SOMEONE!"));
		}
		DrawDebugSphere(GetWorld(), Location, 80.f, 12, FColor::Orange, false, 4.f);
	}
}

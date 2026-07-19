#include "TdfGenerator.h"
#include "TdfGameMode.h"
#include "TdfGameState.h"
#include "TdfKillerCharacter.h"
#include "TdfRunnerCharacter.h"
#include "TdfRunners.h"
#include "TdfTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ATdfGenerator::ATdfGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GeneratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GeneratorMesh"));
	SetRootComponent(GeneratorMesh);
	GeneratorMesh->SetRelativeScale3D(FVector(1.6f, 1.6f, 2.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		GeneratorMesh->SetStaticMesh(MeshAsset.Object);
	}

	// The beacon: a thin pillar reaching into the sky, visible across the whole map.
	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	BeaconMesh->SetupAttachment(GeneratorMesh);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Parent is scaled (1.6,1.6,2) — compensate in relative scale; ~3m wide, ~500m tall.
	BeaconMesh->SetRelativeScale3D(FVector(1.9f, 1.9f, 250.f));
	BeaconMesh->SetRelativeLocation(FVector(0.f, 0.f, 12500.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BeaconAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (BeaconAsset.Succeeded())
	{
		BeaconMesh->SetStaticMesh(BeaconAsset.Object);
	}
}

bool ATdfGenerator::ShouldShowBeaconLocally() const
{
	if (bRepaired)
	{
		return false; // finished generators stop calling for help
	}
	const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
	if (!GameState || GameState->Phase != ETdfMatchPhase::Repair)
	{
		return false;
	}
	// Non-hardcore: beacons for everyone.
	if (GameState->Difficulty != ETdfDifficulty::Hardcore)
	{
		return true;
	}
	// Hardcore: only Dheer/Julius/Mahnam sense them — or whoever looted a finder device.
	const APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	const ATdfRunnerCharacter* Runner = LocalPC ? Cast<ATdfRunnerCharacter>(LocalPC->GetPawn()) : nullptr;
	if (!Runner)
	{
		return false; // killers (and spectators) get nothing in hardcore
	}
	// Dheer's watch senses generators even with a dead battery — but only while he's checking it.
	if (const ATdfRunner_Dheer* Dheer = Cast<ATdfRunner_Dheer>(Runner))
	{
		return Dheer->bWatchHeld;
	}
	if (Runner->RunnerRole == ETdfRunnerRole::Engineer
		|| Runner->RunnerRole == ETdfRunnerRole::Medic)
	{
		return true;
	}
	return Runner->bHasGeneratorFinder;
}

void ATdfGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Per-viewer beacon visibility (runs on every machine — purely local rendering).
	if (BeaconMesh)
	{
		BeaconMesh->SetVisibility(ShouldShowBeaconLocally());
	}

	if (!HasAuthority() || bRepaired)
	{
		return;
	}

	const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
	if (!GameState || GameState->Phase != ETdfMatchPhase::Repair)
	{
		return;
	}

	// Design: 2 players per generator — one repairs (needs the kit), one stands lookout.
	// Exception: the last runner standing may work alone.
	float Rate = 0.f;
	int32 CrewNearby = 0;
	int32 LivingRunners = 0;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (!Runner || Runner->IsDead() || Runner->HasEscaped())
		{
			continue;
		}
		++LivingRunners;
		if (Runner->bIsDown)
		{
			continue;
		}
		const float Dist = FVector::Dist(Runner->GetActorLocation(), GetActorLocation());
		if (Dist <= RepairRadius * 1.8f)
		{
			++CrewNearby; // close enough to count as the lookout
		}
		if (Runner->IsInteracting() && Runner->bHasRepairKit && Dist <= RepairRadius)
		{
			// Julius fixes things 3x faster; Dheer takes 3x longer (see runners.md).
			Rate += (Runner->RunnerRole == ETdfRunnerRole::Engineer) ? 3.f
				: (Runner->RunnerRole == ETdfRunnerRole::Scout) ? (1.f / 3.f) : 1.f;
		}
	}
	if (CrewNearby < 2 && LivingRunners >= 2)
	{
		Rate = 0.f; // no lookout, no progress
	}

	if (Rate > 0.f)
	{
		Progress += Rate * DeltaSeconds;
		if (Progress >= RequiredRepairSeconds)
		{
			Progress = RequiredRepairSeconds;
			bRepaired = true;

			if (ATdfGameMode* GameMode = GetWorld()->GetAuthGameMode<ATdfGameMode>())
			{
				GameMode->NotifyGeneratorRepaired();
			}
		}
	}
}

void ATdfGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATdfGenerator, Progress);
	DOREPLIFETIME(ATdfGenerator, bRepaired);
}

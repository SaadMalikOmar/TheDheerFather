#include "TdfFibreBox.h"
#include "TdfGameMode.h"
#include "TdfGameState.h"
#include "TdfRunnerCharacter.h"
#include "TdfTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ATdfFibreBox::ATdfFibreBox()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	SetRootComponent(BoxMesh);
	BoxMesh->SetRelativeScale3D(FVector(0.8f, 0.5f, 1.2f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		BoxMesh->SetStaticMesh(MeshAsset.Object);
	}

	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	BeaconMesh->SetupAttachment(BoxMesh);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeaconMesh->SetRelativeScale3D(FVector(1.2f, 1.9f, 125.f));
	BeaconMesh->SetRelativeLocation(FVector(0.f, 0.f, 6300.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BeaconAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (BeaconAsset.Succeeded())
	{
		BeaconMesh->SetStaticMesh(BeaconAsset.Object);
	}
}

void ATdfFibreBox::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
	const bool bStageOne = GameState && GameState->Phase == ETdfMatchPhase::FixInternet;

	if (BeaconMesh)
	{
		BeaconMesh->SetVisibility(bStageOne && !bRepaired);
	}

	if (!HasAuthority() || bRepaired || !bStageOne)
	{
		return;
	}

	float Rate = 0.f;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (Runner && !Runner->bIsDown && !Runner->IsDead() && Runner->IsInteracting()
			&& FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= RepairRadius)
		{
			Rate += (Runner->RunnerRole == ETdfRunnerRole::Engineer) ? 3.f
				: (Runner->RunnerRole == ETdfRunnerRole::Scout) ? (1.f / 3.f) : 1.f;
		}
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
				GameMode->NotifyFibreRepaired();
			}
		}
	}
}

void ATdfFibreBox::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfFibreBox, Progress);
	DOREPLIFETIME(ATdfFibreBox, bRepaired);
}

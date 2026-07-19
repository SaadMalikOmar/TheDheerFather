#include "TdfHouseActors.h"
#include "TdfCharacterBase.h"
#include "TdfGameMode.h"
#include "TdfGameState.h"
#include "TdfTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

// --- Seat ---

ATdfSeat::ATdfSeat()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SeatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeatMesh"));
	SetRootComponent(SeatMesh);
	SeatMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		SeatMesh->SetStaticMesh(CubeAsset.Object);
	}

	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	BeaconMesh->SetupAttachment(SeatMesh);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeaconMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 360.f));
	BeaconMesh->SetRelativeLocation(FVector(0.f, 0.f, 18200.f));
	BeaconMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylAsset.Succeeded())
	{
		BeaconMesh->SetStaticMesh(CylAsset.Object);
	}
}

void ATdfSeat::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BeaconMesh && bIsMonitorSeat)
	{
		const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
		BeaconMesh->SetVisibility(GameState && GameState->Phase == ETdfMatchPhase::ReturnHome && !Occupant.IsValid());
	}
}

// --- Router ---

ATdfRouter::ATdfRouter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RouterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RouterMesh"));
	SetRootComponent(RouterMesh);
	RouterMesh->SetRelativeScale3D(FVector(0.45f, 0.16f, 0.16f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		RouterMesh->SetStaticMesh(CubeAsset.Object);
	}

	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	BeaconMesh->SetupAttachment(RouterMesh);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeaconMesh->SetRelativeScale3D(FVector(1.4f, 4.f, 1150.f));
	BeaconMesh->SetRelativeLocation(FVector(0.f, 0.f, 57500.f));
	BeaconMesh->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylAsset.Succeeded())
	{
		BeaconMesh->SetStaticMesh(CylAsset.Object);
	}
}

void ATdfRouter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BeaconMesh)
	{
		const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
		BeaconMesh->SetVisibility(GameState && GameState->Phase == ETdfMatchPhase::CheckRouter && !bChecked);
	}
}

void ATdfRouter::CheckRouter(ATdfCharacterBase* Checker)
{
	if (!HasAuthority() || bChecked)
	{
		return;
	}
	const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
	if (!GameState || GameState->Phase != ETdfMatchPhase::CheckRouter)
	{
		return;
	}
	bChecked = true;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("\"...yep. Internet's dead. It's not just us.\""));
	}
	if (ATdfGameMode* GameMode = GetWorld()->GetAuthGameMode<ATdfGameMode>())
	{
		GameMode->NotifyRouterChecked();
	}
}

void ATdfRouter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATdfRouter, bChecked);
}

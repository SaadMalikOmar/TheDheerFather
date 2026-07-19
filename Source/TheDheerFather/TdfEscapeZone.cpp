#include "TdfEscapeZone.h"
#include "TdfGameState.h"
#include "TdfRunnerCharacter.h"
#include "TdfTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ATdfEscapeZone::ATdfEscapeZone()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

	ZoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneMesh"));
	SetRootComponent(ZoneMesh);
	ZoneMesh->SetRelativeScale3D(FVector(10.f, 10.f, 0.15f));
	ZoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	auto MakePart = [this](const TCHAR* Name, const FVector& RelLoc, const FVector& Scale) {
		UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Comp->SetupAttachment(ZoneMesh);
		Comp->SetRelativeLocation(RelLoc);
		Comp->SetRelativeScale3D(Scale);
		if (MeshAsset.Succeeded()) { Comp->SetStaticMesh(MeshAsset.Object); }
		return Comp;
	};

	// Striped roadworks barriers (relative to the 10x10x0.15 pad scale).
	BarrierA = MakePart(TEXT("BarrierA"), FVector(-35.f, 0.f, 400.f), FVector(0.02f, 0.28f, 7.f));
	BarrierB = MakePart(TEXT("BarrierB"), FVector(35.f, 0.f, 400.f), FVector(0.02f, 0.28f, 7.f));
	BarrierC = MakePart(TEXT("BarrierC"), FVector(0.f, -35.f, 400.f), FVector(0.28f, 0.02f, 7.f));
	// The mains switch they all flip together.
	SwitchBox = MakePart(TEXT("SwitchBox"), FVector(0.f, 0.f, 500.f), FVector(0.06f, 0.04f, 9.f));

	if (MeshAsset.Succeeded())
	{
		ZoneMesh->SetStaticMesh(MeshAsset.Object);
	}
}

void ATdfEscapeZone::BeginPlay()
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
		Tint(ZoneMesh, FLinearColor(0.1f, 0.5f, 0.15f));
		Tint(BarrierA, FLinearColor(1.f, 0.45f, 0.f)); // roadworks orange
		Tint(BarrierB, FLinearColor(1.f, 0.45f, 0.f));
		Tint(BarrierC, FLinearColor(1.f, 0.45f, 0.f));
		Tint(SwitchBox, FLinearColor(0.75f, 0.05f, 0.05f));
	}
}

void ATdfEscapeZone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
	if (!GameState || GameState->Phase != ETdfMatchPhase::Escape)
	{
		return;
	}

	// The design: EVERYONE still alive gathers here, then they flip the power together.
	// (Runners tied to Tung Tung's shrine can't come — they don't count, they need rescuing.)
	int32 Alive = 0, Present = 0;
	TArray<ATdfRunnerCharacter*> Crew;
	for (TActorIterator<ATdfRunnerCharacter> It(GetWorld()); It; ++It)
	{
		ATdfRunnerCharacter* Runner = *It;
		if (!Runner || Runner->IsDead() || Runner->HasEscaped() || Runner->bTiedToShrine)
		{
			continue;
		}
		++Alive;
		if (FVector::Dist(Runner->GetActorLocation(), GetActorLocation()) <= EscapeRadius)
		{
			++Present;
			Crew.Add(Runner);
		}
	}

	if (Present != LastAnnouncedPresent && Alive > 0)
	{
		LastAnnouncedPresent = Present;
		if (Present > 0 && Present < Alive && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(11, 3.f, FColor::Green,
				FString::Printf(TEXT("ROADWORKS: %d/%d waiting... everyone alive must be here"), Present, Alive));
		}
	}

	if (Alive > 0 && Present >= Alive)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Green, TEXT("*CLUNK* ...THE LIGHTS COME BACK ON. YOU MADE IT."));
		}
		for (ATdfRunnerCharacter* Runner : Crew)
		{
			Runner->Escape(); // the last one triggers the win check
		}
	}
}

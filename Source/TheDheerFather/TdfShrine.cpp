#include "TdfShrine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ATdfShrine::ATdfShrine()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	SetRootComponent(BaseMesh);
	BaseMesh->SetRelativeScale3D(FVector(4.5f, 4.5f, 0.4f));

	PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
	PillarMesh->SetupAttachment(BaseMesh);
	PillarMesh->SetRelativeScale3D(FVector(0.16f, 0.16f, 11.f));
	PillarMesh->SetRelativeLocation(FVector(0.f, 0.f, 570.f));
	PillarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		BaseMesh->SetStaticMesh(CubeAsset.Object);
		PillarMesh->SetStaticMesh(CubeAsset.Object);
	}
}

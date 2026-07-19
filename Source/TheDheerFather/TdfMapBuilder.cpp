#include "TdfMapBuilder.h"
#include "TdfWaddonData.h"
#include "TdfGameInstance.h"
#include "TdfGameMode.h"
#include "TdfGameState.h"
#include "TdfGenerator.h"
#include "TdfEscapeZone.h"
#include "TdfFibreBox.h"
#include "TdfHouseActors.h"
#include "TdfDeployables.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float CmPerM = 100.f; // metres -> cm
}

ATdfMapBuilder::ATdfMapBuilder()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	auto MakeISM = [this](const TCHAR* Name) {
		UInstancedStaticMeshComponent* ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);
		ISM->SetupAttachment(GetRootComponent());
		ISM->SetMobility(EComponentMobility::Static);
		return ISM;
	};

	RoadISM = MakeISM(TEXT("RoadISM"));
	GroundISM = MakeISM(TEXT("GroundISM"));
	WallISM = MakeISM(TEXT("WallISM"));
	TreeISM = MakeISM(TEXT("TreeISM"));
	WaterISM = MakeISM(TEXT("WaterISM"));
	MarkerISM = MakeISM(TEXT("MarkerISM"));
	FurnitureISM = MakeISM(TEXT("FurnitureISM"));

	if (CubeAsset.Succeeded())
	{
		RoadISM->SetStaticMesh(CubeAsset.Object);
		GroundISM->SetStaticMesh(CubeAsset.Object);
		WallISM->SetStaticMesh(CubeAsset.Object);
		WaterISM->SetStaticMesh(CubeAsset.Object);
		FurnitureISM->SetStaticMesh(CubeAsset.Object);
	}
	if (CylinderAsset.Succeeded())
	{
		TreeISM->SetStaticMesh(CylinderAsset.Object);
		MarkerISM->SetStaticMesh(CylinderAsset.Object);
	}
	MarkerISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMatAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMatAsset.Succeeded())
	{
		GreyboxBaseMaterial = BaseMatAsset.Object;
	}
}

void ATdfMapBuilder::ApplyGreyboxColors()
{
	if (!GreyboxBaseMaterial)
	{
		return;
	}
	auto Tint = [this](UInstancedStaticMeshComponent* ISM, const FLinearColor& Color)
	{
		if (ISM)
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(GreyboxBaseMaterial, this);
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			ISM->SetMaterial(0, MID);
		}
	};
	Tint(RoadISM, FLinearColor(0.10f, 0.10f, 0.12f));   // asphalt
	Tint(GroundISM, FLinearColor(0.28f, 0.36f, 0.22f)); // grass
	Tint(WallISM, FLinearColor(0.55f, 0.42f, 0.32f));   // brick
	Tint(TreeISM, FLinearColor(0.10f, 0.30f, 0.12f));   // foliage
	Tint(WaterISM, FLinearColor(0.12f, 0.30f, 0.60f));  // water
	Tint(MarkerISM, FLinearColor(1.0f, 0.05f, 0.05f));  // killer-house marker
	Tint(FurnitureISM, FLinearColor(0.42f, 0.30f, 0.18f)); // wood
}

void ATdfMapBuilder::LoadMapData()
{
	// Whyteleafe Hill, #188 (top mini-roundabout, index 0) down to #78 (Badgers Walk). 760 m.
	// Extracted from OpenStreetMap; metres, origin at segment midpoint. X = east, Y = north.
	Road = {
		{ -80.8f, -367.1f, 0.0f },
		{ -83.1f, -359.8f, 7.7f },
		{ -83.5f, -344.2f, 23.3f },
		{ -82.0f, -284.5f, 83.0f },
		{ -78.2f, -241.8f, 125.9f },
		{ -72.4f, -200.2f, 167.9f },
		{ -60.7f, -143.6f, 225.6f },
		{ -59.0f, -138.1f, 231.4f },
		{ -41.5f, -82.0f, 290.3f },
		{ -3.9f, 18.4f, 397.4f },
		{ 24.2f, 86.4f, 471.0f },
		{ 34.0f, 112.6f, 498.9f },
		{ 50.8f, 155.7f, 545.2f },
		{ 63.1f, 196.2f, 587.6f },
		{ 71.8f, 238.2f, 630.4f },
		{ 78.2f, 280.9f, 673.6f },
		{ 79.9f, 309.8f, 702.5f },
		{ 83.5f, 367.1f, 760.0f },
	};
	SegLen = 760.f;

	Bldgs = {
		{ 114.5f, 322.0f, -93.3f, 16.8f, 8.4f, 716.8f, 0.f },
		{ 145.4f, 352.2f, -125.1f, 47.0f, 8.8f, 748.9f, 0.f },
		{ 105.5f, 305.1f, -118.1f, 9.1f, 7.9f, 699.3f, 0.f },
		{ 96.4f, 297.3f, -95.8f, 8.8f, 7.8f, 691.0f, 0.f },
		{ 76.5f, 160.7f, -105.7f, 23.1f, 11.8f, 557.5f, 0.f },
	};

	Waters.Empty();
	ExtraRoads.Empty();
	Greens.Empty();
	RealTrees.Empty();
	KillerHousePosM = FVector2D::ZeroVector;
	// GasStationPosM intentionally NOT reset: it's per-instance editable. Whyteleafe's
	// default is none (run dry = tough luck); BearCampus sets one in its level.
}

FVector2D ATdfMapBuilder::NearestRoadPointTo(float XM, float YM) const
{
	FVector2D Best(0.f, 0.f);
	float BestD2 = 1e18f;
	for (const FTdfRoadPt& P : Road)
	{
		const float D2 = FMath::Square(P.X - XM) + FMath::Square(P.Y - YM);
		if (D2 < BestD2) { BestD2 = D2; Best = FVector2D(P.X, P.Y); }
	}
	for (const TArray<FVector2D>& Poly : ExtraRoads)
	{
		for (const FVector2D& P : Poly)
		{
			const float D2 = FMath::Square(P.X - XM) + FMath::Square(P.Y - YM);
			if (D2 < BestD2) { BestD2 = D2; Best = P; }
		}
	}
	return Best;
}

void ATdfMapBuilder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildGeometry();
}

void ATdfMapBuilder::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	if (HasAuthority())
	{
		// Server rolls the round layout seed; clients rebuild the identical street from it.
		if (ATdfGameState* GameState = GetWorld()->GetGameState<ATdfGameState>())
		{
			if (GameState->MapRoundSeed == 0)
			{
				GameState->MapRoundSeed = FMath::RandRange(1, 999999);
			}
		}
		BuildGeometry();
		bRuntimeBuilt = true;
		if (!bGameplayActorsPlaced)
		{
			PlaceGameplayActors();
		}
	}
	else
	{
		// Client: wait for the replicated seed, then build the matching layout.
		SetActorTickEnabled(true);
	}
}

void ATdfMapBuilder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bRuntimeBuilt && !HasAuthority())
	{
		const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr;
		if (GameState && GameState->MapRoundSeed != 0)
		{
			BuildGeometry();
			bRuntimeBuilt = true;
			SetActorTickEnabled(false);
		}
	}
}

float ATdfMapBuilder::ZAtD(float D) const
{
	// Index 0 is the TOP of the hill; Z falls as you head down.
	return (SegLen - D) * HillGrade * CmPerM;
}

void ATdfMapBuilder::SampleRoad(float D, FVector& OutPos, FVector2D& OutDir) const
{
	if (Road.Num() < 2)
	{
		OutPos = FVector::ZeroVector;
		OutDir = FVector2D(1.f, 0.f);
		return;
	}
	D = FMath::Clamp(D, 0.f, SegLen);
	int32 Idx = 0;
	while (Idx < Road.Num() - 2 && Road[Idx + 1].D < D)
	{
		++Idx;
	}
	const FTdfRoadPt& A = Road[Idx];
	const FTdfRoadPt& B = Road[Idx + 1];
	const float T = (B.D > A.D) ? (D - A.D) / (B.D - A.D) : 0.f;

	OutPos = FVector(FMath::Lerp(A.X, B.X, T) * CmPerM, FMath::Lerp(A.Y, B.Y, T) * CmPerM, ZAtD(D));
	OutDir = FVector2D(B.X - A.X, B.Y - A.Y).GetSafeNormal();
}

void ATdfMapBuilder::AddBoxInstance(UInstancedStaticMeshComponent* ISM, const FVector& CenterCm, const FRotator& Rot, const FVector& SizeM)
{
	if (ISM)
	{
		ISM->AddInstance(FTransform(Rot, CenterCm, FVector(SizeM.X, SizeM.Y, SizeM.Z)), true);
	}
}

void ATdfMapBuilder::AddHouse(const FVector& CenterCm, float YawDeg, float LenM, float WidM, float HeightM, const FVector& RoadPosCm, bool bEnterable)
{
	const FRotator Rot(0.f, YawDeg, 0.f);
	const FVector Fwd = Rot.RotateVector(FVector::ForwardVector);
	const FVector Side = Rot.RotateVector(FVector::RightVector);

	const float WallT = 0.3f;
	const float HalfH = HeightM * 0.5f * CmPerM;
	const FVector WallZ(0.f, 0.f, HalfH);

	const FVector WallAPos = CenterCm + Side * (WidM * 0.5f * CmPerM);
	const FVector WallBPos = CenterCm - Side * (WidM * 0.5f * CmPerM);
	const bool bAFacesRoad = FVector::DistSquared2D(WallAPos, RoadPosCm) < FVector::DistSquared2D(WallBPos, RoadPosCm);

	// Human-scale doorway: 1.3 m wide, 2.1 m tall, with a header above it.
	const float DoorW = 1.3f;
	const float DoorH = 2.1f;
	const float SegLenM = (LenM - DoorW) * 0.5f;
	const FVector FrontPos = bAFacesRoad ? WallAPos : WallBPos;
	const FVector BackPos = bAFacesRoad ? WallBPos : WallAPos;

	if (bEnterable)
	{
		AddBoxInstance(WallISM, FrontPos + Fwd * ((DoorW * 0.5f + SegLenM * 0.5f) * CmPerM) + WallZ, Rot, FVector(SegLenM, WallT, HeightM));
		AddBoxInstance(WallISM, FrontPos - Fwd * ((DoorW * 0.5f + SegLenM * 0.5f) * CmPerM) + WallZ, Rot, FVector(SegLenM, WallT, HeightM));
		if (HeightM > DoorH + 0.3f)
		{
			const float HeaderH = HeightM - DoorH;
			AddBoxInstance(WallISM, FrontPos + FVector(0.f, 0.f, (DoorH + HeaderH * 0.5f) * CmPerM), Rot, FVector(DoorW + 0.1f, WallT, HeaderH));
		}
	}
	else
	{
		// Most people lock their doors when the street goes dark.
		AddBoxInstance(WallISM, FrontPos + WallZ, Rot, FVector(LenM, WallT, HeightM));
	}
	AddBoxInstance(WallISM, BackPos + WallZ, Rot, FVector(LenM, WallT, HeightM));
	AddBoxInstance(WallISM, CenterCm + Fwd * (LenM * 0.5f * CmPerM) + WallZ, Rot, FVector(WallT, WidM, HeightM));
	AddBoxInstance(WallISM, CenterCm - Fwd * (LenM * 0.5f * CmPerM) + WallZ, Rot, FVector(WallT, WidM, HeightM));
	AddBoxInstance(WallISM, CenterCm + FVector(0.f, 0.f, 5.f), Rot, FVector(LenM + 0.6f, WidM + 0.6f, 0.15f));
	AddBoxInstance(WallISM, CenterCm + FVector(0.f, 0.f, HeightM * CmPerM), Rot, FVector(LenM + 0.6f, WidM + 0.6f, 0.2f));
}

void ATdfMapBuilder::BuildStarterHouse(const FVector& BaseCm, float YawDeg, const FVector& RoadPosCm)
{
	const FRotator Rot(0.f, YawDeg, 0.f);
	auto L = [&](float X, float Y, float Z) {
		return BaseCm + Rot.RotateVector(FVector(X * CmPerM, Y * CmPerM, Z * CmPerM));
	};

	// Shell: 11 x 9 m, two storeys, door always open (it's YOUR house).
	AddHouse(BaseCm, YawDeg, 11.f, 9.f, 5.6f, RoadPosCm, true);

	// First floor slab at 2.6 m with a stair opening in the back-left corner.
	AddBoxInstance(FurnitureISM, L(0.f, 1.4f, 2.6f), Rot, FVector(11.f, 6.2f, 0.16f));
	AddBoxInstance(FurnitureISM, L(2.7f, -3.3f, 2.6f), Rot, FVector(5.6f, 2.8f, 0.16f));

	// The stairs: a ramp rising to the opening.
	AddBoxInstance(FurnitureISM, L(-2.4f, -3.3f, 1.25f), FRotator(-29.f, YawDeg, 0.f), FVector(5.6f, 1.5f, 0.14f));

	// Ground floor: sofa, TV, kitchen counter.
	AddBoxInstance(FurnitureISM, L(2.6f, 2.6f, 0.4f), Rot, FVector(2.3f, 0.95f, 0.75f));
	AddBoxInstance(FurnitureISM, L(2.6f, 4.05f, 1.0f), Rot, FVector(1.5f, 0.18f, 0.85f));
	AddBoxInstance(FurnitureISM, L(-3.2f, 3.9f, 0.5f), Rot, FVector(2.5f, 0.65f, 0.95f));

	// Upstairs bedroom: bed, desk, monitor; wardrobe to hide in.
	AddBoxInstance(FurnitureISM, L(3.3f, 3.2f, 3.05f), Rot, FVector(2.1f, 1.6f, 0.55f));
	AddBoxInstance(FurnitureISM, L(-2.7f, 3.85f, 3.15f), Rot, FVector(1.8f, 0.8f, 0.78f));
	AddBoxInstance(FurnitureISM, L(-2.7f, 4.05f, 3.85f), Rot, FVector(0.7f, 0.13f, 0.45f));
	AddBoxInstance(FurnitureISM, L(3.9f, -3.5f, 3.9f), Rot, FVector(1.4f, 1.0f, 2.4f));

	// Interactables + this player's spawn, recorded for PlaceGameplayActors.
	MonitorSeatSpawns.Add(FTransform(FRotator(0.f, YawDeg + 90.f, 0.f), L(-2.7f, 2.9f, 2.95f)));
	HideSpotSpawns.Add(FTransform(FRotator(0.f, YawDeg, 0.f), L(3.9f, -3.5f, 3.0f)));
	RouterSpawns.Add(FTransform(FRotator(0.f, YawDeg, 0.f), L(-4.7f, 1.2f, 1.25f)));
	StarterPlayerStarts.Add(FTransform(FRotator(0.f, YawDeg + 90.f, 0.f), L(-1.5f, 2.2f, 4.0f)));
	RepairKitSpawns.Add(L(2.8f, -3.0f, 0.55f)); // stage 2: grab it before hunting generators
	StarterHomeCenters.Add(FVector2D(BaseCm.X / CmPerM, BaseCm.Y / CmPerM));
	AllHouseCenters.Add(FVector2D(BaseCm.X / CmPerM, BaseCm.Y / CmPerM));
}

void ATdfMapBuilder::BuildGeometry()
{
	if (!RoadISM || !GroundISM || !WallISM || !TreeISM)
	{
		return;
	}
	LoadMapData();
	if (Road.Num() < 2)
	{
		return;
	}

	RoadISM->ClearInstances();
	GroundISM->ClearInstances();
	WallISM->ClearInstances();
	TreeISM->ClearInstances();
	if (WaterISM) { WaterISM->ClearInstances(); }
	if (MarkerISM) { MarkerISM->ClearInstances(); }
	if (FurnitureISM) { FurnitureISM->ClearInstances(); }
	MonitorSeatSpawns.Reset();
	HideSpotSpawns.Reset();
	RouterSpawns.Reset();
	StarterPlayerStarts.Reset();
	StarterHomeCenters.Reset();
	LootFoodSpawns.Reset();
	LootWeaponSpawns.Reset();
	LootWeaponTypes.Reset();
	RepairKitSpawns.Reset();
	AllHouseCenters.Reset();
	ApplyGreyboxColors();

	// Per-round layout: mix the map seed with the server-rolled round seed (identical on all machines).
	int32 LayoutSeed = GeometrySeed;
	if (const ATdfGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATdfGameState>() : nullptr)
	{
		if (GameState->MapRoundSeed != 0)
		{
			LayoutSeed = GeometrySeed ^ (GameState->MapRoundSeed * 7919);
		}
	}
	FRandomStream HouseRand(LayoutSeed + 31);

	const float SideSign = bHousesOnRightSide ? 1.f : -1.f;

	// Flat streets (no real hill) get one clean tiled ground plane instead of pitched
	// strips — strips splay and overlap horribly on tightly curved roads.
	const bool bFlatGround = HillGrade < 0.01f;
	const float FlatZ = ZAtD(SegLen * 0.5f);

	// --- Road surface (always follows the curve; flush with the ground on flat maps) ---
	for (int32 i = 0; i < Road.Num() - 1; ++i)
	{
		const FTdfRoadPt& A = Road[i];
		const FTdfRoadPt& B = Road[i + 1];
		const FVector PA(A.X * CmPerM, A.Y * CmPerM, bFlatGround ? FlatZ : ZAtD(A.D));
		const FVector PB(B.X * CmPerM, B.Y * CmPerM, bFlatGround ? FlatZ : ZAtD(B.D));

		const FVector Mid = (PA + PB) * 0.5f;
		const float HorizLen = FVector::Dist2D(PA, PB);
		if (HorizLen < 1.f)
		{
			continue;
		}
		const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(PB.Y - PA.Y, PB.X - PA.X));
		const float Pitch = FMath::RadiansToDegrees(FMath::Atan2(PB.Z - PA.Z, HorizLen));
		const FRotator Rot(Pitch, Yaw, 0.f);
		const float LenM = HorizLen / CmPerM + 6.f;

		AddBoxInstance(RoadISM, Mid, Rot, FVector(LenM, RoadWidthM, 0.25f));

		if (!bFlatGround)
		{
			const FVector Right = Rot.RotateVector(FVector::RightVector);
			const float GroundOff = (RoadWidthM * 0.5f + GroundStripWidthM * 0.5f) * CmPerM;
			AddBoxInstance(GroundISM, Mid + Right * GroundOff - FVector(0, 0, 12.f), Rot, FVector(LenM + 6.f, GroundStripWidthM, 0.25f));
			AddBoxInstance(GroundISM, Mid - Right * GroundOff - FVector(0, 0, 12.f), Rot, FVector(LenM + 6.f, GroundStripWidthM, 0.25f));
		}
	}

	// --- The rest of the district's streets (flat strips) ---
	for (const TArray<FVector2D>& Poly : ExtraRoads)
	{
		for (int32 i = 0; i < Poly.Num() - 1; ++i)
		{
			const FVector2D& A = Poly[i];
			const FVector2D& B = Poly[i + 1];
			const float LenM = FVector2D::Distance(A, B);
			if (LenM < 1.f || LenM > 300.f)
			{
				continue;
			}
			const FVector2D Mid2 = (A + B) * 0.5f;
			const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(B.Y - A.Y, B.X - A.X));
			AddBoxInstance(RoadISM, FVector(Mid2.X * CmPerM, Mid2.Y * CmPerM, FlatZ), FRotator(0.f, Yaw, 0.f), FVector(LenM + 3.f, RoadWidthM * 0.85f, 0.25f));
		}
	}

	// --- Flat maps: tiled ground covering the whole play area, with the pond as a real basin ---
	float AreaMinX = 1e9f, AreaMaxX = -1e9f, AreaMinY = 1e9f, AreaMaxY = -1e9f;
	{
		auto Grow = [&](float X, float Y) {
			AreaMinX = FMath::Min(AreaMinX, X); AreaMaxX = FMath::Max(AreaMaxX, X);
			AreaMinY = FMath::Min(AreaMinY, Y); AreaMaxY = FMath::Max(AreaMaxY, Y);
		};
		for (const FTdfRoadPt& P : Road) { Grow(P.X, P.Y); }
		for (const TArray<FVector2D>& Poly : ExtraRoads) { for (const FVector2D& P : Poly) { Grow(P.X, P.Y); } }
		for (const FTdfBakedBldg& B : Bldgs) { Grow(B.CX, B.CY); }
		for (const FTdfWaterBox& W : Waters) { Grow(W.CX - W.W * 0.5f, W.CY - W.H * 0.5f); Grow(W.CX + W.W * 0.5f, W.CY + W.H * 0.5f); }
		for (const FTdfWaterBox& G : Greens) { Grow(G.CX - G.W * 0.5f, G.CY - G.H * 0.5f); Grow(G.CX + G.W * 0.5f, G.CY + G.H * 0.5f); }
	}

	if (bFlatGround)
	{
		float MinX = AreaMinX, MaxX = AreaMaxX, MinY = AreaMinY, MaxY = AreaMaxY;
		const float Margin = bPerimeterWalls ? 30.f : GroundStripWidthM + 60.f;
		MinX -= Margin; MaxX += Margin; MinY -= Margin; MaxY += Margin;

		const float TileM = 25.f;
		const float GroundZ = ZAtD(SegLen * 0.5f) - 14.f;
		for (float TX = MinX; TX < MaxX; TX += TileM)
		{
			for (float TY = MinY; TY < MaxY; TY += TileM)
			{
				const float CX = TX + TileM * 0.5f;
				const float CY = TY + TileM * 0.5f;

				// Skip tiles inside the pond — it becomes a walk-in basin instead.
				bool bInWater = false;
				for (const FTdfWaterBox& W : Waters)
				{
					if (FMath::Abs(CX - W.CX) < W.W * 0.5f && FMath::Abs(CY - W.CY) < W.H * 0.5f)
					{
						bInWater = true;
						break;
					}
				}
				if (!bInWater)
				{
					AddBoxInstance(GroundISM, FVector(CX * CmPerM, CY * CmPerM, GroundZ), FRotator::ZeroRotator, FVector(TileM + 0.4f, TileM + 0.4f, 0.25f));
				}
			}
		}

		// Pond basin floors (blue, shallow enough to hop back out of).
		for (const FTdfWaterBox& W : Waters)
		{
			AddBoxInstance(WaterISM, FVector(W.CX * CmPerM, W.CY * CmPerM, GroundZ - 90.f), FRotator::ZeroRotator, FVector(W.W + TileM, W.H + TileM, 0.25f));
		}

		// Perimeter: wall the whole district in (the play-area boundary).
		if (bPerimeterWalls)
		{
			const float WallH = 12.f;
			const float SpanX = MaxX - MinX;
			const float SpanY = MaxY - MinY;
			const float MidX = (MinX + MaxX) * 0.5f;
			const float MidY = (MinY + MaxY) * 0.5f;
			const FVector ZUp(0.f, 0.f, GroundZ + WallH * 0.5f * CmPerM);
			AddBoxInstance(WallISM, FVector(MidX * CmPerM, MinY * CmPerM, 0.f) + ZUp, FRotator::ZeroRotator, FVector(SpanX + 2.f, 1.f, WallH));
			AddBoxInstance(WallISM, FVector(MidX * CmPerM, MaxY * CmPerM, 0.f) + ZUp, FRotator::ZeroRotator, FVector(SpanX + 2.f, 1.f, WallH));
			AddBoxInstance(WallISM, FVector(MinX * CmPerM, MidY * CmPerM, 0.f) + ZUp, FRotator::ZeroRotator, FVector(1.f, SpanY + 2.f, WallH));
			AddBoxInstance(WallISM, FVector(MaxX * CmPerM, MidY * CmPerM, 0.f) + ZUp, FRotator::ZeroRotator, FVector(1.f, SpanY + 2.f, WallH));
		}
	}

	// --- Road-closure barriers at both ends (area maps use perimeter walls instead) ---
	if (!bPerimeterWalls)
	{
		FVector Pos; FVector2D Dir;
		SampleRoad(4.f, Pos, Dir);
		float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
		AddBoxInstance(RoadISM, Pos + FVector(0, 0, 150.f), FRotator(0.f, Yaw, 0.f), FVector(2.f, RoadWidthM + 6.f, 3.f));
		SampleRoad(SegLen - 4.f, Pos, Dir);
		Yaw = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
		AddBoxInstance(RoadISM, Pos + FVector(0, 0, 150.f), FRotator(0.f, Yaw, 0.f), FVector(2.f, RoadWidthM + 6.f, 3.f));
	}

	// Loot roll shared by loot houses everywhere: 45% food, 30% a weapon (rarity by strength).
	auto RollLoot = [&](const FVector& HouseCenter)
	{
		if (HouseRand.FRand() < 0.45f)
		{
			LootFoodSpawns.Add(HouseCenter + FVector(HouseRand.FRandRange(-150.f, 150.f), HouseRand.FRandRange(-150.f, 150.f), 45.f));
		}
		if (HouseRand.FRand() < 0.30f)
		{
			const float Roll = HouseRand.FRand();
			const uint8 Type = (Roll < 0.5f) ? (uint8)ETdfWeaponType::Taser
				: (Roll < 0.8f) ? (uint8)ETdfWeaponType::Axon : (uint8)ETdfWeaponType::TripleTBat;
			LootWeaponSpawns.Add(HouseCenter + FVector(HouseRand.FRandRange(-150.f, 150.f), HouseRand.FRandRange(-150.f, 150.f), 60.f));
			LootWeaponTypes.Add(Type);
		}
	};

	// --- Houses: 3 types — spawn homes (random spots), loot houses (~30%), locked houses ---
	struct FHouseSlot { FVector Center; float Yaw; FVector RoadPos; };
	TArray<FHouseSlot> Slots;
	{
		FVector LastCenter(1e12f, 1e12f, 0.f);
		for (float D = 25.f; D < SegLen - 25.f; D += HouseSpacingM)
		{
			FVector RoadPos; FVector2D Dir;
			SampleRoad(D, RoadPos, Dir);
			const FVector2D RightPerp(Dir.Y, -Dir.X);
			const FVector2D Offset2D = RightPerp * SideSign * HouseOffsetM;
			const FVector Center = RoadPos + FVector(Offset2D.X * CmPerM, Offset2D.Y * CmPerM, 0.f);
			if (FVector::Dist2D(Center, LastCenter) < 16.5f * CmPerM)
			{
				continue;
			}
			bool bBlocked = false;
			for (const FTdfBakedBldg& BD : Bldgs)
			{
				if (FMath::Square(Center.X - BD.CX * CmPerM) + FMath::Square(Center.Y - BD.CY * CmPerM) < FMath::Square(14.f * CmPerM))
				{
					bBlocked = true;
					break;
				}
			}
			if (!bBlocked && !KillerHousePosM.IsZero()
				&& FMath::Square(Center.X - KillerHousePosM.X * CmPerM) + FMath::Square(Center.Y - KillerHousePosM.Y * CmPerM) < FMath::Square(16.f * CmPerM))
			{
				bBlocked = true;
			}
			if (bBlocked)
			{
				continue;
			}
			Slots.Add({ Center, (float)FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X)), RoadPos });
			LastCenter = Center;
		}
	}
	// Shuffle: spawn homes land in different spots every round.
	for (int32 i = Slots.Num() - 1; i > 0; --i)
	{
		Slots.Swap(i, HouseRand.RandRange(0, i));
	}
	int32 SlotIdx = 0;
	for (int32 i = 0; i < NumStarterHomes && SlotIdx < Slots.Num(); ++i, ++SlotIdx)
	{
		BuildStarterHouse(Slots[SlotIdx].Center, Slots[SlotIdx].Yaw, Slots[SlotIdx].RoadPos);
	}
	for (; bProceduralHouses && SlotIdx < Slots.Num(); ++SlotIdx)
	{
		// SEMI-DETACHED: every slot is a PAIR of dwellings sharing a wall — each rolled
		// separately as a loot house or a locked one. ~110 front doors up the hill.
		const FHouseSlot& S = Slots[SlotIdx];
		const FVector Along = FRotator(0.f, S.Yaw, 0.f).RotateVector(FVector::ForwardVector);
		for (int32 Unit = 0; Unit < 2; ++Unit)
		{
			const FVector UnitCenter = S.Center + Along * ((Unit == 0 ? -1.f : 1.f) * 4.05f * CmPerM);
			const bool bLoot = HouseRand.FRand() < LootHouseChance;
			AddHouse(UnitCenter, S.Yaw, 8.f, 8.5f, WallHeightM, S.RoadPos, bLoot);
			AllHouseCenters.Add(FVector2D(UnitCenter.X / CmPerM, UnitCenter.Y / CmPerM));
			if (bLoot)
			{
				RollLoot(UnitCenter);
			}
		}
	}

	// --- Real OSM footprints: same loot/locked split ---
	for (const FTdfBakedBldg& BD : Bldgs)
	{
		const FVector2D Near = NearestRoadPointTo(BD.CX, BD.CY);
		const FVector RoadPos(Near.X * CmPerM, Near.Y * CmPerM, ZAtD(BD.D));
		const float H = (BD.H > 0.f) ? BD.H : WallHeightM;
		const FVector Center(BD.CX * CmPerM, BD.CY * CmPerM, ZAtD(BD.D));
		const bool bLoot = HouseRand.FRand() < LootHouseChance;
		AddHouse(Center, BD.Ang, BD.Len, BD.Wid, H, RoadPos, bLoot);
		AllHouseCenters.Add(FVector2D(BD.CX, BD.CY));
		if (bLoot)
		{
			RollLoot(Center);
		}
	}

	// --- The killer's house (only built if a real footprint isn't already standing there) ---
	if (!KillerHousePosM.IsZero() && bShowKillerMarker && MarkerISM)
	{
		// Big red pillar so you can find the killer house from anywhere (testing aid).
		MarkerISM->AddInstance(FTransform(FRotator::ZeroRotator,
			FVector(KillerHousePosM.X * CmPerM, KillerHousePosM.Y * CmPerM, ZAtD(SegLen * 0.5f) + 3000.f),
			FVector(1.2f, 1.2f, 60.f)), true);
	}
	if (!KillerHousePosM.IsZero())
	{
		bool bRealHouseThere = false;
		for (const FTdfBakedBldg& BD : Bldgs)
		{
			if (FMath::Square(BD.CX - KillerHousePosM.X) + FMath::Square(BD.CY - KillerHousePosM.Y) < FMath::Square(14.f))
			{
				bRealHouseThere = true;
				break;
			}
		}
		if (!bRealHouseThere)
		{
			const FVector2D Near = NearestRoadPointTo(KillerHousePosM.X, KillerHousePosM.Y);
			AddHouse(FVector(KillerHousePosM.X * CmPerM, KillerHousePosM.Y * CmPerM, ZAtD(SegLen * 0.5f)), KillerHouseYaw, 12.f, 10.f, 6.f,
				FVector(Near.X * CmPerM, Near.Y * CmPerM, ZAtD(SegLen * 0.5f)));
		}
		AllHouseCenters.Add(KillerHousePosM);
	}

	// (Procedural street houses are generated above via the shuffled slot system.)

	// --- Trees — deterministic, identical on all machines ---
	FRandomStream Rand(GeometrySeed);

	// Real surveyed trees first (exact positions from map data).
	for (const FVector2D& TreePos : RealTrees)
	{
		const float TrunkH = Rand.FRandRange(7.f, 12.f);
		const float TrunkR = Rand.FRandRange(0.4f, 0.8f);
		const float BaseZ = ZAtD(SegLen * 0.5f);
		TreeISM->AddInstance(FTransform(FRotator::ZeroRotator,
			FVector(TreePos.X * CmPerM, TreePos.Y * CmPerM, BaseZ + TrunkH * 0.5f * CmPerM - 12.f),
			FVector(TrunkR, TrunkR, TrunkH)), true);
		const float CanopyR = Rand.FRandRange(2.6f, 4.2f);
		const float CanopyH = Rand.FRandRange(3.5f, 5.5f);
		TreeISM->AddInstance(FTransform(FRotator::ZeroRotator,
			FVector(TreePos.X * CmPerM, TreePos.Y * CmPerM, BaseZ + (TrunkH - CanopyH * 0.3f) * CmPerM),
			FVector(CanopyR, CanopyR, CanopyH)), true);
	}
	int32 Placed = 0, Attempts = 0;
	while (Placed < TreeCount && Attempts < TreeCount * 6)
	{
		++Attempts;
		FVector2D TreePos2D;
		float BaseZ;

		if (Greens.Num() > 0)
		{
			// Area maps: trees live in the real parks and green patches.
			const FTdfWaterBox& G = Greens[Rand.RandRange(0, Greens.Num() - 1)];
			TreePos2D.X = G.CX + Rand.FRandRange(-0.5f, 0.5f) * G.W;
			TreePos2D.Y = G.CY + Rand.FRandRange(-0.5f, 0.5f) * G.H;
			BaseZ = ZAtD(SegLen * 0.5f);
		}
		else
		{
			// Street maps: forest belt on the green side of the road.
			const float D = Rand.FRandRange(5.f, SegLen - 5.f);
			const float Off = Rand.FRandRange(RoadWidthM * 0.5f + 8.f, GroundStripWidthM - 10.f);
			FVector RoadPos; FVector2D Dir;
			SampleRoad(D, RoadPos, Dir);
			const FVector2D LeftPerp = FVector2D(Dir.Y, -Dir.X) * -SideSign;
			TreePos2D = FVector2D(RoadPos.X / CmPerM, RoadPos.Y / CmPerM) + LeftPerp * Off;
			BaseZ = RoadPos.Z;
		}

		// Not in the pond, not on a road, not inside somebody's living room.
		bool bRejected = false;
		for (const FTdfWaterBox& W : Waters)
		{
			if (FMath::Abs(TreePos2D.X - W.CX) < W.W * 0.5f && FMath::Abs(TreePos2D.Y - W.CY) < W.H * 0.5f)
			{
				bRejected = true;
				break;
			}
		}
		if (!bRejected && FVector2D::Distance(NearestRoadPointTo(TreePos2D.X, TreePos2D.Y), TreePos2D) < RoadWidthM * 0.5f + 4.f)
		{
			bRejected = true;
		}
		if (!bRejected)
		{
			for (const FTdfBakedBldg& B : Bldgs)
			{
				if (FMath::Square(TreePos2D.X - B.CX) + FMath::Square(TreePos2D.Y - B.CY) < FMath::Square(10.f))
				{
					bRejected = true;
					break;
				}
			}
		}
		if (!bRejected && !KillerHousePosM.IsZero()
			&& FVector2D::DistSquared(TreePos2D, KillerHousePosM) < FMath::Square(13.f))
		{
			bRejected = true;
		}
		if (bRejected)
		{
			continue;
		}

		const float TrunkH = Rand.FRandRange(7.f, 13.f);
		const float TrunkR = Rand.FRandRange(0.4f, 0.8f);
		const FVector Base(TreePos2D.X * CmPerM, TreePos2D.Y * CmPerM, BaseZ + TrunkH * 0.5f * CmPerM - 12.f);
		TreeISM->AddInstance(FTransform(FRotator::ZeroRotator, Base, FVector(TrunkR, TrunkR, TrunkH)), true);

		// Canopy: a fat cylinder sitting on the trunk so trees read as trees (and hide runners).
		const float CanopyR = Rand.FRandRange(2.6f, 4.2f);
		const float CanopyH = Rand.FRandRange(3.5f, 5.5f);
		const FVector CanopyPos(TreePos2D.X * CmPerM, TreePos2D.Y * CmPerM,
			BaseZ + (TrunkH - CanopyH * 0.3f) * CmPerM);
		TreeISM->AddInstance(FTransform(FRotator::ZeroRotator, CanopyPos, FVector(CanopyR, CanopyR, CanopyH)), true);
		++Placed;
	}
}

void ATdfMapBuilder::PlaceGameplayActors()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}
	bGameplayActorsPlaced = true;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const float SideSign = bHousesOnRightSide ? 1.f : -1.f;

	// Starter homes: spawn in your bedroom, at your setup. Otherwise: kerbside spawns.
	if (StarterPlayerStarts.Num() > 0)
	{
		for (const FTransform& T : StarterPlayerStarts)
		{
			World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), T.GetLocation(), T.Rotator(), Params);
		}
		for (const FTransform& T : MonitorSeatSpawns)
		{
			if (ATdfSeat* Seat = World->SpawnActor<ATdfSeat>(ATdfSeat::StaticClass(), T.GetLocation(), T.Rotator(), Params))
			{
				Seat->bIsMonitorSeat = true;
			}
		}
		for (const FTransform& T : HideSpotSpawns)
		{
			if (ATdfSeat* Hide = World->SpawnActor<ATdfSeat>(ATdfSeat::StaticClass(), T.GetLocation(), T.Rotator(), Params))
			{
				Hide->bHideSpot = true;
				Hide->SetActorHiddenInGame(true); // the wardrobe shell is the visual
			}
		}
		for (const FTransform& T : RouterSpawns)
		{
			World->SpawnActor<ATdfRouter>(ATdfRouter::StaticClass(), T.GetLocation(), T.Rotator(), Params);
		}
		for (const FVector& T : RepairKitSpawns)
		{
			World->SpawnActor<ATdfRepairKit>(ATdfRepairKit::StaticClass(), T, FRotator::ZeroRotator, Params);
		}
	}
	else
	{
		for (int32 i = 0; i < NumPlayerStarts; ++i)
		{
			const float D = SegLen * (i + 1) / (NumPlayerStarts + 1);
			FVector Pos; FVector2D Dir;
			SampleRoad(D, Pos, Dir);
			const FVector2D RightPerp(Dir.Y, -Dir.X);
			const FVector2D Off = RightPerp * SideSign * (RoadWidthM * 0.5f + 3.f);
			World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
				Pos + FVector(Off.X * CmPerM, Off.Y * CmPerM, 250.f), FRotator::ZeroRotator, Params);
		}
	}

	// Loot-house drops (rolled during geometry, identical layout everywhere).
	for (const FVector& FoodPos : LootFoodSpawns)
	{
		World->SpawnActor<ATdfFoodPickup>(ATdfFoodPickup::StaticClass(), FoodPos, FRotator::ZeroRotator, Params);
	}
	for (int32 i = 0; i < LootWeaponSpawns.Num(); ++i)
	{
		if (ATdfWeaponPickup* Pickup = World->SpawnActor<ATdfWeaponPickup>(ATdfWeaponPickup::StaticClass(),
			LootWeaponSpawns[i], FRotator::ZeroRotator, Params))
		{
			Pickup->WeaponType = (ETdfWeaponType)LootWeaponTypes[i];
		}
	}

	// The killer's own spawn (inside the killer house if this map has one).
	if (!KillerHousePosM.IsZero())
	{
		World->SpawnActor<ATdfKillerStart>(ATdfKillerStart::StaticClass(),
			FVector(KillerHousePosM.X * CmPerM, KillerHousePosM.Y * CmPerM, ZAtD(SegLen * 0.5f) + 250.f),
			FRotator::ZeroRotator, Params);
	}

	// MOST HATED S.O — the petrol station (maps that have one).
	if (!GasStationPosM.IsZero())
	{
		World->SpawnActor<ATdfGasStation>(ATdfGasStation::StaticClass(),
			FVector(GasStationPosM.X * CmPerM, GasStationPosM.Y * CmPerM, ZAtD(SegLen * 0.5f) + 12.f),
			FRotator::ZeroRotator, Params);
	}

	if (bAutoPlaceObjectives)
	{
		// The party-lobby leader can override how many generators spawn.
		if (const UTdfGameInstance* GI = World->GetGameInstance<UTdfGameInstance>())
		{
			if (GI->GeneratorCountOverride > 0)
			{
				NumGenerators = GI->GeneratorCountOverride;
			}
		}

		FRandomStream Rand(FMath::Rand());

		// Objectives never spawn inside a house (that was the "buggy generators" culprit).
		auto ClearOfHouses = [&](const FVector2D& PosM)
		{
			for (const FVector2D& House : AllHouseCenters)
			{
				if (FVector2D::DistSquared(PosM, House) < FMath::Square(9.5f))
				{
					return false;
				}
			}
			return true;
		};

		for (int32 i = 0; i < NumGenerators; ++i)
		{
			// Truly scattered: random spot along the map, random side, kerb to deep forest.
			const float Band = (SegLen - 120.f) / NumGenerators;
			FVector SpawnPos = FVector::ZeroVector;
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				const float D = 60.f + Band * i + Rand.FRandRange(0.f, Band * 0.85f);
				FVector Pos; FVector2D Dir;
				SampleRoad(D, Pos, Dir);
				const FVector2D RightPerp(Dir.Y, -Dir.X);
				const float SideRoll = (Rand.FRand() < 0.5f) ? 1.f : -1.f;
				const FVector2D Off = RightPerp * SideRoll * Rand.FRandRange(RoadWidthM * 0.5f + 4.f, GroundStripWidthM - 14.f);
				SpawnPos = Pos + FVector(Off.X * CmPerM, Off.Y * CmPerM, 120.f);
				if (ClearOfHouses(FVector2D(SpawnPos.X / CmPerM, SpawnPos.Y / CmPerM)))
				{
					break;
				}
			}
			World->SpawnActor<ATdfGenerator>(ATdfGenerator::StaticClass(), SpawnPos, FRotator::ZeroRotator, Params);
		}

		const float ExitD = FMath::RandBool() ? 12.f : SegLen - 12.f;
		FVector Pos; FVector2D Dir;
		SampleRoad(ExitD, Pos, Dir);
		World->SpawnActor<ATdfEscapeZone>(ATdfEscapeZone::StaticClass(), Pos + FVector(0, 0, 30.f), FRotator::ZeroRotator, Params);

		// Stage 1 fibre boxes: off the road, away from the houses (design: street or forest).
		for (int32 i = 0; i < NumFibreBoxes; ++i)
		{
			const float Band = (SegLen - 100.f) / FMath::Max(1, NumFibreBoxes);
			FVector SpawnPos = FVector::ZeroVector;
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				const float D = 50.f + Band * i + Rand.FRandRange(0.f, Band * 0.7f);
				FVector FPos; FVector2D FDir;
				SampleRoad(D, FPos, FDir);
				const FVector2D RightPerp(FDir.Y, -FDir.X);
				const float FSide = (Rand.FRand() < 0.5f) ? 1.f : -1.f;
				const FVector2D Off = RightPerp * FSide * Rand.FRandRange(12.f, 45.f);
				SpawnPos = FPos + FVector(Off.X * CmPerM, Off.Y * CmPerM, 90.f);
				if (ClearOfHouses(FVector2D(SpawnPos.X / CmPerM, SpawnPos.Y / CmPerM)))
				{
					break;
				}
			}
			World->SpawnActor<ATdfFibreBox>(ATdfFibreBox::StaticClass(), SpawnPos, FRotator::ZeroRotator, Params);
		}

		// Weapons: scarce pickups scattered along the street.
		for (int32 i = 0; i < NumWeapons; ++i)
		{
			const float D = Rand.FRandRange(30.f, SegLen - 30.f);
			FVector WPos; FVector2D WDir;
			SampleRoad(D, WPos, WDir);
			const FVector2D RightPerp(WDir.Y, -WDir.X);
			const FVector2D Off = RightPerp * SideSign * Rand.FRandRange(4.f, 22.f);
			ATdfWeaponPickup* Pickup = World->SpawnActor<ATdfWeaponPickup>(ATdfWeaponPickup::StaticClass(),
				WPos + FVector(Off.X * CmPerM, Off.Y * CmPerM, 60.f), FRotator(0.f, Rand.FRandRange(0.f, 360.f), 0.f), Params);
			if (Pickup)
			{
				const int32 Roll = Rand.RandRange(0, 2);
				Pickup->WeaponType = (Roll == 0) ? ETdfWeaponType::Axon
					: (Roll == 1) ? ETdfWeaponType::TripleTBat : ETdfWeaponType::Taser;
			}
		}

		if (ATdfGameMode* GameMode = World->GetAuthGameMode<ATdfGameMode>())
		{
			GameMode->RefreshObjectiveCount();
		}
	}
}

// =========================== WONDERPOND (the whole Waddon district) ===========================

ATdfMapBuilder_Waddon::ATdfMapBuilder_Waddon()
{
	// Croydon is flat; the real parks replace the forest belt.
	HillGrade = 0.004f;
	TreeCount = 260;
	NumPlayerStarts = 6;
	NumWeapons = 7;
	NumStarterHomes = 7; // full Load Shedding flow here too
	bHousesOnRightSide = false;
}

void ATdfMapBuilder_Waddon::LoadMapData()
{
	// The whole Waddon district from OpenStreetMap (see TdfWaddonData.h, generated):
	// every street, all 129 real buildings, Waddon Ponds, and the parks.
	Road.Empty();
	for (int32 i = 0; i < UE_ARRAY_COUNT(GWaddonMainRoad); ++i)
	{
		Road.Add({ GWaddonMainRoad[i][0], GWaddonMainRoad[i][1], GWaddonMainRoad[i][2] });
	}
	SegLen = GWaddonSegLen;

	ExtraRoads.Empty();
	for (int32 r = 0; r < UE_ARRAY_COUNT(GWaddonRoadRanges); ++r)
	{
		const int32 Start = GWaddonRoadRanges[r][0];
		const int32 Count = GWaddonRoadRanges[r][1];
		TArray<FVector2D>& Poly = ExtraRoads.AddDefaulted_GetRef();
		Poly.Reserve(Count);
		for (int32 i = Start; i < Start + Count; ++i)
		{
			Poly.Add(FVector2D(GWaddonRoadPts[i][0], GWaddonRoadPts[i][1]));
		}
	}

	Bldgs.Empty();
	for (int32 i = 0; i < UE_ARRAY_COUNT(GWaddonBldgs); ++i)
	{
		Bldgs.Add({ GWaddonBldgs[i][0], GWaddonBldgs[i][1], GWaddonBldgs[i][2],
			GWaddonBldgs[i][3], GWaddonBldgs[i][4], GWaddonSegLen * 0.5f, GWaddonBldgs[i][5] });
	}

	Waters.Empty();
	for (int32 i = 0; i < UE_ARRAY_COUNT(GWaddonWaters); ++i)
	{
		Waters.Add({ GWaddonWaters[i][0], GWaddonWaters[i][1], GWaddonWaters[i][2], GWaddonWaters[i][3] });
	}

	Greens.Empty();
	for (int32 i = 0; i < UE_ARRAY_COUNT(GWaddonGreens); ++i)
	{
		Greens.Add({ GWaddonGreens[i][0], GWaddonGreens[i][1], GWaddonGreens[i][2], GWaddonGreens[i][3] });
	}

	// 23 Waddon Court Road — the killer's house (a real footprint; the start spawns inside it).
	KillerHousePosM = FVector2D(GWaddonKillerX, GWaddonKillerY);
	KillerHouseYaw = 90.f;

	// MOST HATED S.O sits exactly where the real Esso is on Purley Way.
	GasStationPosM = FVector2D(76.0f, 98.4f);

	// Every individually surveyed tree in the district.
	RealTrees.Empty();
	for (int32 i = 0; i < UE_ARRAY_COUNT(GWaddonRealTrees); ++i)
	{
		RealTrees.Add(FVector2D(GWaddonRealTrees[i][0], GWaddonRealTrees[i][1]));
	}

	// Real footprints cover the whole district; wall it in as the play area.
	bProceduralHouses = false;
	bPerimeterWalls = true;
}

// (Whitewood keeps a MOST HATED S.O near the trail start — set in its LoadMapData below.)

// =========================== PARTY LOBBY ISLAND ===========================

ATdfMapBuilder_Lobby::ATdfMapBuilder_Lobby()
{
	HillGrade = 0.001f;
	TreeCount = 14;          // a bit of decoration to climb while waiting
	NumPlayerStarts = 6;
	NumWeapons = 0;
	NumFibreBoxes = 0;
	NumGenerators = 0;
	NumStarterHomes = 0;
	bAutoPlaceObjectives = false;
	bShowKillerMarker = false;
}

void ATdfMapBuilder_Lobby::LoadMapData()
{
	Road.Empty();
	ExtraRoads.Empty();
	Bldgs.Empty();
	Waters.Empty();
	Greens.Empty();
	KillerHousePosM = FVector2D::ZeroVector;

	// A short promenade — the flat-ground tiling grows a comfy island around it.
	Road = { { -30.f, 0.f, 0.f }, { 30.f, 0.f, 60.f } };
	SegLen = 60.f;

	// One clubhouse to run around in.
	Bldgs = { { 0.f, 28.f, 0.f, 14.f, 10.f, 30.f, 0.f } };

	// Trees around the edges.
	Greens = { { 0.f, -30.f, 90.f, 24.f } };

	bProceduralHouses = false;
	bPerimeterWalls = true;
}

void ATdfMapBuilder_Lobby::PlaceGameplayActors()
{
	Super::PlaceGameplayActors();

	// Mark this world as the front-end lobby: H launches the selected map from here.
	if (HasAuthority())
	{
		if (ATdfGameState* TdfState = GetWorld()->GetGameState<ATdfGameState>())
		{
			TdfState->bFrontEndLobby = true;
		}
	}
}

// =========================== WHITEWOOD (procedural forest) ===========================

ATdfMapBuilder_Forest::ATdfMapBuilder_Forest()
{
	HillGrade = 0.004f;   // gentle enough to stay in flat-ground mode
	TreeCount = 1400;     // properly dense — you should lose sight of the trail
	NumPlayerStarts = 8;
	NumWeapons = 6;
	NumStarterHomes = 7; // cabins with setups — full Load Shedding out here too
}

void ATdfMapBuilder_Forest::LoadMapData()
{
	Road.Empty();
	ExtraRoads.Empty();
	Bldgs.Empty();
	Waters.Empty();
	Greens.Empty();

	FRandomStream R(GeometrySeed * 7 + 101);

	// --- A winding dirt trail heading roughly north, ~650 m ---
	float X = 0.f, Y = -330.f, D = 0.f, Heading = 90.f;
	Road.Add({ X, Y, 0.f });
	for (int32 i = 0; i < 22; ++i)
	{
		Heading = FMath::Clamp(Heading + R.FRandRange(-26.f, 26.f), 48.f, 132.f);
		const float Step = R.FRandRange(26.f, 36.f);
		X += FMath::Cos(FMath::DegreesToRadians(Heading)) * Step;
		Y += FMath::Sin(FMath::DegreesToRadians(Heading)) * Step;
		D += Step;
		Road.Add({ X, Y, D });
	}
	SegLen = D;

	// --- Cabins in clearings, alternating sides of the trail ---
	auto TrailPoint = [this](float AtD, FVector2D& OutPos, FVector2D& OutDir)
	{
		FVector PosCm; FVector2D Dir;
		SampleRoad(AtD, PosCm, Dir);
		OutPos = FVector2D(PosCm.X / 100.f, PosCm.Y / 100.f);
		OutDir = Dir;
	};

	const int32 NumCabins = 13;
	for (int32 i = 0; i < NumCabins; ++i)
	{
		const float AtD = 50.f + (SegLen - 100.f) * i / (NumCabins - 1) + R.FRandRange(-14.f, 14.f);
		FVector2D Pos, Dir;
		TrailPoint(AtD, Pos, Dir);
		const FVector2D Perp(Dir.Y, -Dir.X);
		const float Side = (R.FRand() < 0.5f) ? 1.f : -1.f;
		const FVector2D Center = Pos + Perp * Side * R.FRandRange(16.f, 30.f);
		const float Yaw = (float)FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X)) + R.FRandRange(-20.f, 20.f);
		Bldgs.Add({ (float)Center.X, (float)Center.Y, Yaw, (float)R.FRandRange(9.f, 12.f), (float)R.FRandRange(7.5f, 9.5f), SegLen * 0.5f, 0.f });
	}

	// --- The lake, off-trail near the middle ---
	{
		FVector2D Pos, Dir;
		TrailPoint(SegLen * 0.45f, Pos, Dir);
		const FVector2D Perp(Dir.Y, -Dir.X);
		const FVector2D LakeC = Pos + Perp * 75.f;
		Waters.Add({ (float)LakeC.X, (float)LakeC.Y, 58.f, 44.f });
	}

	// --- The killer's lodge, deep off-trail in the north woods ---
	{
		FVector2D Pos, Dir;
		TrailPoint(SegLen * 0.74f, Pos, Dir);
		const FVector2D Perp(Dir.Y, -Dir.X);
		KillerHousePosM = Pos - Perp * R.FRandRange(36.f, 46.f);
		KillerHouseYaw = R.FRandRange(0.f, 360.f);
	}

	// --- The forest itself: one big green blanket over the whole area ---
	{
		float MinX = 1e9f, MaxX = -1e9f, MinY = 1e9f, MaxY = -1e9f;
		for (const FTdfRoadPt& P : Road)
		{
			MinX = FMath::Min(MinX, P.X); MaxX = FMath::Max(MaxX, P.X);
			MinY = FMath::Min(MinY, P.Y); MaxY = FMath::Max(MaxY, P.Y);
		}
		const float Pad = 85.f;
		Greens.Add({ (MinX + MaxX) * 0.5f, (MinY + MaxY) * 0.5f, (MaxX - MinX) + Pad * 2.f, (MaxY - MinY) + Pad * 2.f });
	}

	// A lonely MOST HATED S.O by the trailhead.
	GasStationPosM = FVector2D(18.f, -310.f);

	bProceduralHouses = false;
	bPerimeterWalls = true;
}

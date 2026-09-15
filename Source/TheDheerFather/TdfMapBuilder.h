// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "TdfMapBuilder.generated.h"

class UInstancedStaticMeshComponent;

/** Marks where the killer spawns (e.g. 21 Waddon Court Road). GameMode prefers this for killers. */
UCLASS()
class THEDHEERFATHER_API ATdfKillerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	ATdfKillerStart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
};

/** Baked map data (metres, origin at segment midpoint). */
struct FTdfRoadPt { float X, Y, D; };
struct FTdfBakedBldg { float CX, CY, Ang, Len, Wid, D; float H; };
struct FTdfWaterBox { float CX, CY, W, H; };

/**
 * Greybox street generator — LOAD SHEDDING map: the real Whyteleafe Hill, #188 (top
 * mini-roundabout) down to #78 (Badgers Walk), 760 m, geometry from OpenStreetMap.
 *
 * Drop ONE into a level at the origin. Geometry builds identically on every machine
 * (instanced meshes, deterministic seed). On the server it also places PlayerStarts,
 * generators and the escape zone (50/50 top or bottom).
 *
 * Subclasses override LoadMapData() to bake other real streets (see ATdfMapBuilder_Waddon).
 */
UCLASS()
class THEDHEERFATHER_API ATdfMapBuilder : public AActor
{
	GENERATED_BODY()

public:
	ATdfMapBuilder();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	/** Hill steepness: Z drops this fraction per metre travelled downhill. */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map", meta = (ClampMin = "0", ClampMax = "0.3"))
	float HillGrade = 0.06f;

	UPROPERTY(EditAnywhere, Category = "Tdf|Map") float RoadWidthM = 8.f;
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") float GroundStripWidthM = 80.f;

	/** Slot spacing for the semi-detached PAIRS lining the street (2 dwellings per slot). */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") float HouseSpacingM = 17.5f;
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") float HouseOffsetM = 16.f;
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") float WallHeightM = 5.f;

	/** Flip if the houses ended up on the wrong side vs real life. */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") bool bHousesOnRightSide = false;

	UPROPERTY(EditAnywhere, Category = "Tdf|Map") int32 TreeCount = 220;
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") int32 GeometrySeed = 1337;

	/** Server: auto-spawn PlayerStarts, generators and the escape zone on BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") bool bAutoPlaceObjectives = true;
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") int32 NumGenerators = 3;
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") int32 NumPlayerStarts = 8;

	/** Weapons scattered around the map (scarce on purpose). */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") int32 NumWeapons = 5;

	/** Stage 1 fibre boxes (Load Shedding intro). 0 = skip straight to the generator stage. */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") int32 NumFibreBoxes = 4;

	/** Furnished two-storey spawn homes (bedroom PC setups, routers, wardrobes, repair kits). */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") int32 NumStarterHomes = 7;

	/** Of the non-spawn houses: this fraction are open LOOT houses; the rest are locked shut. */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map", meta = (ClampMin = "0", ClampMax = "1"))
	float LootHouseChance = 0.30f;

	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Fills Road/Bldgs/Waters/SegLen (+ optional killer house). Base = Whyteleafe Hill. */
	virtual void LoadMapData();

	void BuildGeometry();
	virtual void PlaceGameplayActors();

	void SampleRoad(float D, FVector& OutPos, FVector2D& OutDir) const;
	float ZAtD(float D) const;

	/** Hollow greybox house: 4 walls, floor and roof. Enterable ones get a door gap facing the road. */
	void AddHouse(const FVector& CenterCm, float YawDeg, float LenM, float WidM, float HeightM, const FVector& RoadPosCm, bool bEnterable = true);

	/** A furnished two-storey home: stairs, sofa/TV/kitchen below, bed/desk/PC/wardrobe above. */
	void BuildStarterHouse(const FVector& BaseCm, float YawDeg, const FVector& RoadPosCm);

	void AddBoxInstance(UInstancedStaticMeshComponent* ISM, const FVector& CenterCm, const FRotator& Rot, const FVector& SizeM);

	// Baked data (filled by LoadMapData).
	TArray<FTdfRoadPt> Road;
	TArray<FTdfBakedBldg> Bldgs;
	TArray<FTdfWaterBox> Waters;
	float SegLen = 0.f;

	// Area-map extras: every other street in the district, park rectangles for trees.
	TArray<TArray<FVector2D>> ExtraRoads;
	TArray<FTdfWaterBox> Greens;

	/** Fill gaps with generated semis (off for area maps — real footprints cover everything). */
	bool bProceduralHouses = true;

	/** Wall in the whole district instead of just barricading the main road's ends. */
	bool bPerimeterWalls = false;

	/** Nearest street vertex to a point (metres) — used to face house doors at the street. */
	FVector2D NearestRoadPointTo(float XM, float YM) const;

	/** If set (non-zero), a special killer house is built here and the killer spawns in it. */
	FVector2D KillerHousePosM = FVector2D::ZeroVector;
	float KillerHouseYaw = 0.f;

	/** If set (non-zero), a MOST HATED S.O petrol station spawns here (metres). (Whyteleafe default: none — on purpose.) */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map")
	FVector2D GasStationPosM = FVector2D::ZeroVector;

	/** Individually surveyed real trees (e.g. Waddon's street/park trees from OSM). */
	TArray<FVector2D> RealTrees;

	UPROPERTY(VisibleAnywhere, Category = "Tdf|Map") UInstancedStaticMeshComponent* RoadISM;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Map") UInstancedStaticMeshComponent* GroundISM;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Map") UInstancedStaticMeshComponent* WallISM;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Map") UInstancedStaticMeshComponent* TreeISM;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Map") UInstancedStaticMeshComponent* WaterISM;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Map") UInstancedStaticMeshComponent* MarkerISM;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Map") UInstancedStaticMeshComponent* FurnitureISM;

	/** Interactable/spawn transforms produced by BuildStarterHouse, consumed by PlaceGameplayActors. */
	TArray<FTransform> MonitorSeatSpawns;
	TArray<FTransform> HideSpotSpawns;
	TArray<FTransform> RouterSpawns;
	TArray<FTransform> StarterPlayerStarts;
	TArray<FVector2D> StarterHomeCenters;

	/** Loot-house drops + repair kits (rolled during geometry, spawned server-side). */
	TArray<FVector> LootFoodSpawns;
	TArray<FVector> LootWeaponSpawns;
	TArray<uint8> LootWeaponTypes;
	TArray<FVector> RepairKitSpawns;

	/** Every house centre on the map — objectives refuse to spawn inside them. */
	TArray<FVector2D> AllHouseCenters;

	bool bRuntimeBuilt = false;

	/** Engine basic-shape material — tinted per ISM so the greybox is actually readable. */
	UPROPERTY() UMaterialInterface* GreyboxBaseMaterial;

	/** Marks the killer house with a tall red pillar (testing aid). */
	UPROPERTY(EditAnywhere, Category = "Tdf|Map") bool bShowKillerMarker = true;

	void ApplyGreyboxColors();

	bool bGameplayActorsPlaced = false;
};

/**
 * WONDERPOND — the whole Waddon district, Croydon: every street and real building in the
 * bird's-eye area, Waddon Ponds, the parks, the council block, killer house at 23 Waddon Ct Rd.
 */
UCLASS()
class THEDHEERFATHER_API ATdfMapBuilder_Waddon : public ATdfMapBuilder
{
	GENERATED_BODY()
public:
	ATdfMapBuilder_Waddon();

protected:
	virtual void LoadMapData() override;
};

/**
 * WHITEWOOD — original forest map for the base mode: a winding trail through dense woods,
 * cabins in clearings, a lake, and the killer's lodge deep in the trees. Fully procedural
 * (deterministic from GeometrySeed), so every machine builds the identical forest.
 */
UCLASS()
class THEDHEERFATHER_API ATdfMapBuilder_Forest : public ATdfMapBuilder
{
	GENERATED_BODY()
public:
	ATdfMapBuilder_Forest();

protected:
	virtual void LoadMapData() override;
};

/**
 * PARTY LOBBY ISLAND — the pre-game hangout. Friends join here (F1 host / F2 join),
 * mess around, and the leader picks map / difficulty / generators / killer on-off,
 * then launches with H. No killer, no objectives — just vibes and a platform.
 */
UCLASS()
class THEDHEERFATHER_API ATdfMapBuilder_Lobby : public ATdfMapBuilder
{
	GENERATED_BODY()
public:
	ATdfMapBuilder_Lobby();

protected:
	virtual void LoadMapData() override;
	virtual void PlaceGameplayActors() override;
};

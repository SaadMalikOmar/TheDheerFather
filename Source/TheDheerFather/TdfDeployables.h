#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TdfTypes.h"
#include "TdfDeployables.generated.h"

class UStaticMeshComponent;

/** Musa's decoy: a body-double that stands there looking suspicious for 30s. */
UCLASS()
class THEDHEERFATHER_API ATdfDecoy : public AActor
{
	GENERATED_BODY()

public:
	ATdfDecoy();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* DecoyMesh;
};

/**
 * Lucki's blue Prius, parked in the world. E next to it to sit in (if unlocked),
 * RMB looking at it to lock/unlock. Inside: E puts the keys in / takes them out,
 * RMB cranks the engine (5 seconds) or kills it, Shift gets out.
 * Hybrid drivetrain: fast driving burns petrol (and trickle-charges the battery),
 * slow driving is silent electric. Refuels at a gas station with the engine off.
 */
UCLASS()
class THEDHEERFATHER_API ATdfPriusProp : public AActor
{
	GENERATED_BODY()

public:
	ATdfPriusProp();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Server-side controls (called by Lucki).
	void ToggleLock();
	void ToggleKeys();
	void RequestEngineToggle();

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Car") bool bLocked = false;
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Car") bool bKeysInserted = false;
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Car") bool bEngineOn = false;
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Car") float EngineStartRemaining = 0.f;
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Car") float Petrol = 100.f;
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Car") float Battery = 100.f;

	UPROPERTY(EditAnywhere, Category = "Tdf|Car") float EngineStartSeconds = 5.f;
	UPROPERTY(EditAnywhere, Category = "Tdf|Car") float PetrolBurnPerSecond = 0.9f;
	UPROPERTY(EditAnywhere, Category = "Tdf|Car") float BatteryBurnPerSecond = 0.45f;
	UPROPERTY(EditAnywhere, Category = "Tdf|Car") float RefuelPerSecond = 7.f;

	/** Whoever's behind the wheel (server). */
	TWeakObjectPtr<class ATdfKiller_Lucki> Driver;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* CarMesh;
};

/** The petrol station: "MOST HATED S.O". Park with the engine off to refuel. */
UCLASS()
class THEDHEERFATHER_API ATdfGasStation : public AActor
{
	GENERATED_BODY()

public:
	ATdfGasStation();

	UPROPERTY(EditAnywhere, Category = "Tdf")
	float RefuelRadius = 800.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* PadMesh;
	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* CanopyMesh;
	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* PillarA;
	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* PillarB;
	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* PumpA;
	UPROPERTY(VisibleAnywhere, Category = "Tdf") UStaticMeshComponent* PumpB;
	UPROPERTY(VisibleAnywhere, Category = "Tdf") class UTextRenderComponent* SignText;
};

/**
 * Musa's hidden speaker. Invisible to everyone. When the killer comes near it plays back
 * phantom voices so it sounds like runners are there. (Audio playback itself is a TODO —
 * it needs mic/VOIP capture; for now the killer gets a convincing fake noise cue.)
 */
UCLASS()
class THEDHEERFATHER_API ATdfSpeaker : public AActor
{
	GENERATED_BODY()

public:
	ATdfSpeaker();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Tdf") float TriggerRadius = 800.f;
	UPROPERTY(EditAnywhere, Category = "Tdf") float RetriggerCooldown = 25.f;

	/** How long Musa held the record button (playback length once audio lands). */
	UPROPERTY(EditAnywhere, Category = "Tdf") float RecordedSeconds = 3.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayVoices(FVector Location);

	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* SpeakerMesh;

	float CooldownRemaining = 0.f;
};

/** Food found in loot houses: heals runners (+25) — or feeds a starving Lucki (+40 hunger). */
UCLASS()
class THEDHEERFATHER_API ATdfFoodPickup : public AActor
{
	GENERATED_BODY()

public:
	ATdfFoodPickup();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Tdf") float PickupRadius = 150.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* FoodMesh;
};

/** Stage 2: the repair kit from your house — you can't fix a generator without one. */
UCLASS()
class THEDHEERFATHER_API ATdfRepairKit : public AActor
{
	GENERATED_BODY()

public:
	ATdfRepairKit();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Tdf") float PickupRadius = 160.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* KitMesh;
};

/** A weapon lying in the world (see weapons.md). Runners walk over it to pick it up. */
UCLASS()
class THEDHEERFATHER_API ATdfWeaponPickup : public AActor
{
	GENERATED_BODY()

public:
	ATdfWeaponPickup();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Tdf")
	ETdfWeaponType WeaponType = ETdfWeaponType::Axon;

	UPROPERTY(EditAnywhere, Category = "Tdf")
	float PickupRadius = 160.f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* WeaponMesh;
};

/**
 * Hardcore only: a generator-finder gadget (Dheer's smartwatch / Julius's cyberdeck /
 * Mahnam's phone) dropped where its owner died. Any runner who walks over it gains
 * generator beacons for the rest of the match.
 */
UCLASS()
class THEDHEERFATHER_API ATdfFinderDevice : public AActor
{
	GENERATED_BODY()

public:
	ATdfFinderDevice();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Tdf")
	float PickupRadius = 160.f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* DeviceMesh;
};

/**
 * Lucki's Spirit of Nani: a ring-doorbell trap. When a moving runner comes near,
 * every killer player gets an alert ping at that spot. Troos (Tank) is immune (see runners.md).
 */
UCLASS()
class THEDHEERFATHER_API ATdfNaniTrap : public AActor
{
	GENERATED_BODY()

public:
	ATdfNaniTrap();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Tdf")
	float TriggerRadius = 300.f;

	/** Seconds between alerts from the same trap. */
	UPROPERTY(EditAnywhere, Category = "Tdf")
	float RetriggerCooldown = 5.f;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Alert(FVector Location);

	UPROPERTY(VisibleAnywhere, Category = "Tdf")
	UStaticMeshComponent* TrapMesh;

	float CooldownRemaining = 0.f;
};

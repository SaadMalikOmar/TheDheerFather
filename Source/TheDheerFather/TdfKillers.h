#pragma once

#include "CoreMinimal.h"
#include "TdfKillerCharacter.h"
#include "TdfKillers.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * The three killers (see killers.md). Stats + kits set in constructors.
 * Still to come: Skinny Bear voice mimic (needs audio).
 */

/**
 * Mimic — Skinny Bear. THIRD PERSON. One-shot, fast, CANNOT SEE STILL RUNNERS.
 * E/RMB: Sniff — slows him but reveals everyone within 50 m through walls. R: Echo Scream.
 */
UCLASS()
class THEDHEERFATHER_API ATdfKiller_SkinnyBear : public ATdfKillerCharacter
{
	GENERATED_BODY()
public:
	ATdfKiller_SkinnyBear();

	virtual void Tick(float DeltaSeconds) override;

	/** Server: sniff — slow to half speed for Duration. */
	void StartSniffSlow(float Duration);

	/** While > 0 (Echo Scream / Sniff), stillness no longer hides runners. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|SkinnyBear")
	float RevealTimeRemaining = 0.f;

	/** Runners moving slower than this are invisible to Skinny Bear. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|SkinnyBear")
	float StillnessSpeedThreshold = 20.f;

protected:
	void EndSniffSlow();
	FTimerHandle SniffTimerHandle;

	UPROPERTY(VisibleAnywhere, Category = "Tdf|SkinnyBear") USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|SkinnyBear") UCameraComponent* ThirdPersonCamera;
};

/**
 * Stalker — Lucki. Slow and heavy on foot, so he drives the blue Prius parked next to his
 * spawn (C near it to get in, C to park). Hunger: starving Lucki barely walks; he refuels by
 * feeding on downed runners. Q: Nani trap x4. R: release DJ. T: Diet Coke x3.
 * RMB: whistle — switches DJ between HUNT and HEEL.
 */
UCLASS()
class THEDHEERFATHER_API ATdfKiller_Lucki : public ATdfKillerCharacter
{
	GENERATED_BODY()
public:
	ATdfKiller_Lucki();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ToggleVehicle() override;
	virtual bool CanSprint() const override { return !bInCar; }

	/** Shift in the car = get out. */
	virtual void OnSprintPressed() override;

	/** Server (E): sit in the Prius / toggle the ignition keys. */
	void HandleCarInteract();

	/** RMB: engine (inside) / lock (looking at the car) / whistle at DJ (otherwise). */
	virtual void OnSecondaryAction() override;
	UFUNCTION(Server, Reliable) void ServerToggleDJMode();
	UFUNCTION(Server, Reliable) void ServerCarSecondary();

	UFUNCTION(Server, Reliable)
	void ServerSetCar(bool bNewInCar);

	class ATdfPriusProp* GetCar() const { return ParkedCar.Get(); }

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Lucki")
	bool bInCar = false;

	/** 0..100. Below the threshold Lucki starves and crawls. Feeding on downed runners refills it. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|Lucki")
	float Hunger = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Lucki") float HungerDrainPerSecond = 1.2f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Lucki") float StarvingThreshold = 30.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Lucki") float CarSpeed = 300.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Lucki") float RamSpeedThreshold = 500.f;
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Lucki") float RamRange = 230.f;

	/** How close he must be to the parked Prius to get in. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|Lucki") float CarEnterRange = 400.f;

protected:
	void ApplyCarState();

	/** The car he's wearing while driving (the parked one is a separate ATdfPriusProp). */
	UPROPERTY(VisibleAnywhere, Category = "Tdf|Lucki")
	UStaticMeshComponent* CarMesh;

	TWeakObjectPtr<class ATdfPriusProp> ParkedCar;
};

/**
 * Combatant — Tung Tung. Agile FPS killer. E: TREE FORM — disguises as a tree (one red
 * leaf gives him away), 60s recharge. Q: ground smash. R (only 3 min after spawning):
 * ROOT OF FLESH. T: spare dart. RMB: tranq blow dart. F: carry runners to the shrine.
 */
UCLASS()
class THEDHEERFATHER_API ATdfKiller_TungTung : public ATdfKillerCharacter
{
	GENERATED_BODY()
public:
	ATdfKiller_TungTung();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server: F key — pick up a helpless runner, or deliver the one on his shoulder. */
	void TryCarryOrDeliver();

	/** Server: enter the slender form for Duration seconds. */
	void StartRootOfFlesh(float Duration);

	/** Server: toggle the tree disguise. */
	void SetTreeForm(bool bNewTreeForm);

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|TungTung")
	bool bRootOfFleshActive = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Tdf|TungTung")
	bool bTreeForm = false;

	UPROPERTY(EditDefaultsOnly, Category = "Tdf|TungTung") float RootSpeedMultiplier = 2.5f;

	/** Root of Flesh unlocks this many seconds after he spawns. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|TungTung") float UltimateUnlockSeconds = 180.f;

	/** Carrying slows him to this fraction of base speed. */
	UPROPERTY(EditDefaultsOnly, Category = "Tdf|TungTung") float CarrySpeedFraction = 0.55f;

protected:
	void PickUp(class ATdfRunnerCharacter* Runner);
	void TieToShrine(class ATdfRunnerCharacter* Runner, class ATdfShrine* Shrine);
	void DropCarried();
	void EndRootOfFlesh();

	TWeakObjectPtr<class ATdfRunnerCharacter> CarriedRunner;
	FTimerHandle RootTimerHandle;

	/** The disguise: trunk + canopy + the one red leaf that gives him away. */
	UPROPERTY(VisibleAnywhere, Category = "Tdf|TungTung") UStaticMeshComponent* TreeTrunkMesh;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|TungTung") UStaticMeshComponent* TreeCanopyMesh;
	UPROPERTY(VisibleAnywhere, Category = "Tdf|TungTung") UStaticMeshComponent* RedLeafMesh;
};

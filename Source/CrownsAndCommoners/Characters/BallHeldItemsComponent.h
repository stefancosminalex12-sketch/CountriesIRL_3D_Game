// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallHeldItemsComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
struct FCIRLItemRow;

/**
 *  Shows what a ball holds in its hands (main hand = right, off hand = left) as simple stand-in shapes built from
 *  boxes, cylinders and spheres: a pole with a head for a bill, a blade and crossguard for a sword, a board for a
 *  shield... Sized from the item's length (ReachCm) and shape, so every weapon is visible before its real model
 *  exists. When a real model arrives, it replaces these shapes here and nothing else changes.
 */
UCLASS(ClassGroup=(Ball))
class UBallHeldItemsComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Rebuilds the shapes from what is in the hands now (called when the inventory changes) */
	void Refresh();

	/** Shows this item in the main hand instead of what is equipped there (a dagger drawn from the belt); NAME_None = back to normal */
	void SetMainHandOverride(FName ItemId);

protected:

	virtual void BeginPlay() override;

private:

	/** One hand's item: a holder on the hand (turned so its Z runs along the weapon) and a few shape parts on it */
	struct FHeld
	{
		USceneComponent* Holder = nullptr;
		TArray<UStaticMeshComponent*> Parts;
		int32 Used = 0;
	};

	enum class EPartMesh : uint8 { Cylinder, Cube, Sphere };

	/** Adds one shape to a hand's item: centre and size in cm, in the holder's space (Z along the weapon, grip at 0) */
	void AddPart(FHeld& Hand, EPartMesh Mesh, const FVector& Center, const FVector& Size, const FLinearColor& Color, const FRotator& Rotation = FRotator::ZeroRotator);

	void Build(FHeld& Hand, const FCIRLItemRow* Item, float ThumbSide);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> AllParts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USceneComponent>> Holders;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> SphereMesh;

	FHeld Hands[2];

	FName MainHandOverride;
};

// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallSkeletonComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

/**
 *  Cartoon skeleton left when a ball has decayed to bones: a big skull with hollow sockets, a thick neck,
 *  the thickest part in the center (spine/breastbone) with three chunky ribs, and a small pelvis, lying
 *  on the ground. Built from simple shapes at runtime (only corpses ever need it); the art pass can swap
 *  in a Blender model.
 */
UCLASS(ClassGroup=(Ball), meta=(BlueprintSpawnableComponent))
class UBallSkeletonComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBallSkeletonComponent();

	/** Builds the bones lying on the ground. Parent is the ball's visual root; GroundZ is the ground height in its space. */
	void Show(USceneComponent* Parent, float GroundZ, const FLinearColor& BoneColor);

	bool IsShown() const { return Parts.Num() > 0; }

protected:

	/** Skull diameter in cm: big, it was a countryball after all */
	UPROPERTY(EditAnywhere, Category="Skeleton")
	float SkullSize = 46.f;

	/** Thickness of the ribs and the knobs at their ends */
	UPROPERTY(EditAnywhere, Category="Skeleton")
	FVector2D RibThicknessAndKnob = FVector2D(10.f, 13.f);

private:

	/** Adds a mesh piece; BoneColor null keeps the mesh's own material (the socket shell) */
	UStaticMeshComponent* AddPart(USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor* BoneColor);

	UPROPERTY() TObjectPtr<UStaticMesh> SphereMesh;
	UPROPERTY() TObjectPtr<UStaticMesh> CylinderMesh;
	UPROPERTY() TObjectPtr<UMaterialInterface> EyeMaterial;

	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Parts;
};

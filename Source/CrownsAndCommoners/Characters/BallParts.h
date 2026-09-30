// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"

class AActor;
class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;

/**
 *  Helpers for building ball characters out of simple mesh parts.
 *  Placeholder art uses the engine's basic shapes; final art swaps in Blender meshes.
 */
namespace BallParts
{
	/** Creates a non-colliding visual mesh part. Must be called from the owner's constructor. */
	UStaticMeshComponent* Create(AActor* Owner, FName Name, USceneComponent* Parent, UStaticMesh* Mesh);

	/** Gives a part a flat color (creates a dynamic material instance; call at runtime) */
	void SetColor(UStaticMeshComponent* Part, const FLinearColor& Color);

	/** Engine placeholder shapes (100 cm across, cylinder 100 cm tall, at scale 1). Only valid inside a constructor. */
	UStaticMesh* LoadSphere();
	UStaticMesh* LoadCube();
	UStaticMesh* LoadCylinder();
}

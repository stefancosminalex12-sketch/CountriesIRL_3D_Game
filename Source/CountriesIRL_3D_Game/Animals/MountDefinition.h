// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MountDefinition.generated.h"

class USkeletalMesh;
class UAnimSequence;

/**
 *  Everything that makes one kind of riding animal: its model, animations and how it moves.
 *  Horse tiers from the design (affer, hackney, rouncey, palfrey, courser, destrier) are separate
 *  assets of this type: same code, different numbers and looks.
 */
UCLASS(BlueprintType)
class UMountDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Look")
	TObjectPtr<USkeletalMesh> Mesh;

	/** Rotation of the model so it faces the actor's forward (+X) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Look")
	float MeshYaw = -90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimSequence> WalkAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimSequence> GallopAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimSequence> JumpAnim;

	/** Ground speed (cm/s) at which the walk and gallop animations look right at normal play rate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	float WalkAnimSpeed = 170.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	float GallopAnimSpeed = 750.f;

	/** Speeds in cm/s. A horse walks at ~6-7 km/h; a small medieval horse gallops at ~30-35 km/h. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WalkSpeed = 190.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float GallopSpeed = 900.f;

	/** How fast the horse turns (degrees per second): quick at a walk, wide turns at a gallop */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WalkTurnRate = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float GallopTurnRate = 65.f;

	/** How quickly it picks up and loses speed (cm/s²): horses build up and slow down gradually */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float Acceleration = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float JumpVelocity = 450.f;

	/** Horse stamina spent per second of galloping, and per jump */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float GallopStaminaPerSecond = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float JumpStaminaCost = 20.f;

	/** Collision: the capsule covers the chest; the long body is not fully covered yet */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CapsuleRadius = 42.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CapsuleHalfHeight = 85.f;

	/** How far the head reaches in front of the body's center (cm): the horse stops before its head hits a wall */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float HeadReach = 95.f;

	/** Half the width of the body where the rider's legs hang (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float BodyHalfWidth = 26.f;

	/** Bone in the middle of the back that the saddle sits on, and how far above it the rider sits */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Riding")
	FName SaddleBone = TEXT("Torso2");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Riding")
	float SaddleHeight = 14.f;
};

// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "AnimNodes/AnimNode_RetargetPoseFromMesh.h"
#include "CIRLRetargetAnimInstance.generated.h"

class UIKRetargeter;

/** Runs a single "Retarget Pose From Mesh" node, so no Animation Blueprint is needed */
USTRUCT()
struct FCIRLRetargetAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FCIRLRetargetAnimInstanceProxy() = default;
	FCIRLRetargetAnimInstanceProxy(UAnimInstance* InAnimInstance, FAnimNode_RetargetPoseFromMesh* InRetargetNode)
		: FAnimInstanceProxy(InAnimInstance), RetargetNode(InRetargetNode)
	{
	}

	virtual FAnimNode_Base* GetCustomRootNode() override { return RetargetNode; }
	virtual void GetCustomNodes(TArray<FAnimNode_Base*>& OutNodes) override { OutNodes.Add(RetargetNode); }

	FAnimNode_RetargetPoseFromMesh* RetargetNode = nullptr;
};

/**
 * Copies the pose of another skeletal mesh (e.g. the mannequin that plays all our animations)
 * onto this mesh's different skeleton, using an IK Retargeter asset. Used for outfits.
 */
UCLASS(Transient, NotBlueprintable)
class COUNTRIESIRL_3D_GAME_API UCIRLRetargetAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UCIRLRetargetAnimInstance();

	/** Start copying Source's pose through Retargeter. Source must tick before this mesh. */
	void SetSource(UIKRetargeter* Retargeter, USkeletalMeshComponent* Source);

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

	UPROPERTY(Transient)
	FAnimNode_RetargetPoseFromMesh RetargetNode;
};

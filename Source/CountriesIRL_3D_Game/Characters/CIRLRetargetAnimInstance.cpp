// CountriesIRL 3D Game

#include "Characters/CIRLRetargetAnimInstance.h"
#include "Retargeter/IKRetargeter.h"
#include "Components/SkeletalMeshComponent.h"

UCIRLRetargetAnimInstance::UCIRLRetargetAnimInstance()
{
	// The node reads the source mesh's pose on the game thread before updating
	bUseMultiThreadedAnimationUpdate = false;
	RetargetNode.RetargetFrom = ERetargetSourceMode::CustomSkeletalMeshComponent;
}

void UCIRLRetargetAnimInstance::SetSource(UIKRetargeter* Retargeter, USkeletalMeshComponent* Source)
{
	RetargetNode.IKRetargeterAsset = Retargeter;
	RetargetNode.SourceMeshComponent = Source;
}

FAnimInstanceProxy* UCIRLRetargetAnimInstance::CreateAnimInstanceProxy()
{
	return new FCIRLRetargetAnimInstanceProxy(this, &RetargetNode);
}

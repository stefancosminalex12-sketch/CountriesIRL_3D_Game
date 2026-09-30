// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "HorseAnimInstance.generated.h"

class UAnimSequence;
class UMountDefinition;

/** What the horse's pose is made of this frame: two looping clips blended, plus an optional one-shot */
USTRUCT()
struct FHorseAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FHorseAnimInstanceProxy() = default;
	explicit FHorseAnimInstanceProxy(UAnimInstance* InAnimInstance) : FAnimInstanceProxy(InAnimInstance) {}

	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual bool Evaluate(FPoseContext& Output) override;

	const UAnimSequence* ClipA = nullptr;
	const UAnimSequence* ClipB = nullptr;
	float TimeA = 0.f;
	float TimeB = 0.f;
	/** 0 = only A, 1 = only B */
	float BlendAlpha = 0.f;

	const UAnimSequence* OneShot = nullptr;
	float OneShotTime = 0.f;
	float OneShotWeight = 0.f;
};

/**
 *  Horse animation without an Animation Blueprint: blends idle, walk, trot, canter and gallop by ground speed
 *  (playing faster as the horse speeds up, so hooves don't slide), and plays one-shots like a jump.
 *  Clips and reference speeds come from the horse's UMountDefinition.
 */
UCLASS(Transient, NotBlueprintable)
class UHorseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UHorseAnimInstance();

	/** Plays a clip once over the locomotion (e.g. the jump); bHoldAtEnd keeps its last pose (death) */
	void PlayOneShot(UAnimSequence* Clip, bool bHoldAtEnd = false);

protected:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

private:

	friend struct FHorseAnimInstanceProxy;

	UPROPERTY(Transient)
	TObjectPtr<const UMountDefinition> Definition;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> OneShotClip;

	float IdleTime = 0.f;
	float WalkTime = 0.f;
	float TrotTime = 0.f;
	float CanterTime = 0.f;
	float GallopTime = 0.f;
	float SmoothedSpeed = 0.f;
	float OneShotTime = -1.f;
	bool bHoldOneShot = false;

	/** Seconds to fade a one-shot in and out */
	float OneShotBlend = 0.15f;
};

// CountriesIRL 3D Game

#include "Animals/HorseAnimInstance.h"
#include "Animals/Horse.h"
#include "Animals/MountDefinition.h"
#include "AnimationRuntime.h"
#include "Animation/AnimSequence.h"

namespace
{
	float Wrap(float Time, const UAnimSequence* Clip)
	{
		const float Length = Clip ? Clip->GetPlayLength() : 0.f;
		return Length > 0.f ? FMath::Fmod(Time, Length) : 0.f;
	}

	void SamplePose(const UAnimSequence* Clip, float Time, FPoseContext& Pose)
	{
		if (!Clip)
		{
			Pose.ResetToRefPose();
			return;
		}
		FAnimationPoseData Data(Pose);
		Clip->GetAnimationPose(Data, FAnimExtractContext(static_cast<double>(Time), false, {}, true));
	}
}

UHorseAnimInstance::UHorseAnimInstance()
{
	// Everything is tiny; keep the update on the game thread next to the horse's movement
	bUseMultiThreadedAnimationUpdate = false;
}

FAnimInstanceProxy* UHorseAnimInstance::CreateAnimInstanceProxy()
{
	return new FHorseAnimInstanceProxy(this);
}

void UHorseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	if (const AHorse* Horse = Cast<AHorse>(GetOwningActor()))
	{
		Definition = Horse->GetDefinition();
	}
}

void UHorseAnimInstance::PlayOneShot(UAnimSequence* Clip, bool bHoldAtEnd)
{
	OneShotClip = Clip;
	OneShotTime = Clip ? 0.f : -1.f;
	bHoldOneShot = bHoldAtEnd;
}

void UHorseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Definition)
	{
		return;
	}

	// Ground speed, smoothed so gait changes blend instead of popping
	const AActor* Owner = GetOwningActor();
	const float Speed = Owner ? Owner->GetVelocity().Size2D() : 0.f;
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaSeconds, 8.f);

	// Clips play faster the faster the horse moves (never slower than half speed, so it doesn't look frozen)
	const float WalkRate = FMath::Clamp(SmoothedSpeed / FMath::Max(Definition->WalkAnimSpeed, 1.f), 0.5f, 1.6f);
	const float GallopRate = FMath::Clamp(SmoothedSpeed / FMath::Max(Definition->GallopAnimSpeed, 1.f), 0.6f, 1.4f);
	IdleTime += DeltaSeconds;
	WalkTime += DeltaSeconds * WalkRate;
	GallopTime += DeltaSeconds * GallopRate;

	if (OneShotTime >= 0.f)
	{
		OneShotTime += DeltaSeconds;
		if (OneShotClip && bHoldOneShot)
		{
			// Stays on the last frame (e.g. lying dead)
			OneShotTime = FMath::Min(OneShotTime, OneShotClip->GetPlayLength() - 0.01f);
		}
		else if (!OneShotClip || OneShotTime >= OneShotClip->GetPlayLength())
		{
			OneShotTime = -1.f;
			OneShotClip = nullptr;
		}
	}
}

void FHorseAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);

	const UHorseAnimInstance* Horse = CastChecked<UHorseAnimInstance>(InAnimInstance);
	const UMountDefinition* Definition = Horse->Definition;
	if (!Definition)
	{
		ClipA = ClipB = nullptr;
		return;
	}

	// Standing still -> walk -> gallop: blend the two clips around the current speed
	const float Speed = Horse->SmoothedSpeed;
	const float Walk = FMath::Max(Definition->WalkSpeed, 1.f);
	if (Speed <= Walk)
	{
		ClipA = Definition->IdleAnim;
		ClipB = Definition->WalkAnim;
		TimeA = Wrap(Horse->IdleTime, ClipA);
		TimeB = Wrap(Horse->WalkTime, ClipB);
		BlendAlpha = FMath::SmoothStep(0.f, Walk * 0.6f, Speed);
	}
	else
	{
		ClipA = Definition->WalkAnim;
		ClipB = Definition->GallopAnim;
		TimeA = Wrap(Horse->WalkTime, ClipA);
		TimeB = Wrap(Horse->GallopTime, ClipB);
		// The gallop takes over well before full speed (a canter uses the gallop clip, played slower)
		BlendAlpha = FMath::SmoothStep(Walk * 1.2f, Walk * 2.2f, Speed);
	}

	OneShot = Horse->OneShotTime >= 0.f ? Horse->OneShotClip.Get() : nullptr;
	OneShotTime = FMath::Max(Horse->OneShotTime, 0.f);
	if (OneShot)
	{
		const float Remaining = Horse->bHoldOneShot ? Horse->OneShotBlend : OneShot->GetPlayLength() - OneShotTime;
		OneShotWeight = FMath::Clamp(FMath::Min(OneShotTime, Remaining) / Horse->OneShotBlend, 0.f, 1.f);
	}
}

bool FHorseAnimInstanceProxy::Evaluate(FPoseContext& Output)
{
	if (!ClipA)
	{
		Output.ResetToRefPose();
		return true;
	}

	SamplePose(ClipA, TimeA, Output);
	if (ClipB && BlendAlpha > 0.001f)
	{
		FPoseContext PoseB(Output);
		SamplePose(ClipB, TimeB, PoseB);
		FAnimationPoseData DataA(Output);
		FAnimationRuntime::BlendTwoPosesTogetherInPlace(DataA, FAnimationPoseData(PoseB), 1.f - BlendAlpha);
	}

	if (OneShot && OneShotWeight > 0.001f)
	{
		FPoseContext ShotPose(Output);
		SamplePose(OneShot, OneShotTime, ShotPose);
		FAnimationPoseData DataOut(Output);
		FAnimationRuntime::BlendTwoPosesTogetherInPlace(DataOut, FAnimationPoseData(ShotPose), 1.f - OneShotWeight);
	}
	return true;
}

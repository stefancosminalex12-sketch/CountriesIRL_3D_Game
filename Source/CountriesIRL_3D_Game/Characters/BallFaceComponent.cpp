// CountriesIRL 3D Game

#include "Characters/BallFaceComponent.h"
#include "Characters/BallParts.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float BlinkDuration = 0.16f;

	/** Keeps a thin line visible when the eyes are closed, like drawn countryballs */
	constexpr float MaxLidClosure = 0.94f;

	/** The shell floats just above the body surface so the eyes never z-fight with it */
	constexpr float ShellOffset = 0.6f;

	FBallEyePose MakePose(float Scale, float Upper, float Angle, float Lower, bool bDead = false)
	{
		FBallEyePose Pose;
		Pose.EyeScale = Scale;
		Pose.UpperLid = Upper;
		Pose.UpperLidAngle = Angle;
		Pose.LowerLid = Lower;
		Pose.bDead = bDead;
		return Pose;
	}
}

UBallFaceComponent::UBallFaceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	EmotionPoses.Add(EBallEmotion::Neutral,    MakePose(1.0f,  0.0f,   0.f, 0.0f));
	EmotionPoses.Add(EBallEmotion::Happy,      MakePose(1.0f,  0.0f,   0.f, 0.5f));
	EmotionPoses.Add(EBallEmotion::Sad,        MakePose(0.95f, 0.35f, -22.f, 0.0f));
	EmotionPoses.Add(EBallEmotion::Angry,      MakePose(1.0f,  0.4f,  28.f, 0.0f));
	EmotionPoses.Add(EBallEmotion::Scared,     MakePose(1.3f,  0.0f,   0.f, 0.0f));
	EmotionPoses.Add(EBallEmotion::Tired,      MakePose(1.0f,  0.55f,  0.f, 0.1f));
	EmotionPoses.Add(EBallEmotion::Suspicious, MakePose(1.0f,  0.42f,  6.f, 0.3f));
	EmotionPoses.Add(EBallEmotion::Dead,       MakePose(1.0f,  0.0f,   0.f, 0.0f, true));
}

void UBallFaceComponent::CreateFaceMesh(AActor* Owner, USceneComponent* Parent, UStaticMesh* SphereMesh, float BallRadius)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> EyeMaterial(TEXT("/Game/CountriesIRL/Characters/Materials/M_BallEyes.M_BallEyes"));

	FaceShell = BallParts::Create(Owner, TEXT("FaceShell"), Parent, SphereMesh);
	FaceShell->SetRelativeScale3D(FVector((BallRadius + ShellOffset) / 50.f));
	FaceShell->SetMaterial(0, EyeMaterial.Object);
	FaceShell->SetCastShadow(false);
}

void UBallFaceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (FaceShell)
	{
		FaceMaterial = FaceShell->CreateAndSetMaterialInstanceDynamic(0);
	}

	CurrentPose = EmotionPoses.FindRef(Emotion);
	TimeToNextBlink = FMath::FRandRange(BlinkInterval.X, BlinkInterval.Y);
	PushPoseToMaterial();
}

void UBallFaceComponent::SetOwnerNoSee(bool bNoSee)
{
	if (FaceShell)
	{
		FaceShell->SetOwnerNoSee(bNoSee);
	}
}

void UBallFaceComponent::SetEmotion(EBallEmotion NewEmotion)
{
	if (EmotionPoses.Contains(NewEmotion))
	{
		Emotion = NewEmotion;
	}
}

void UBallFaceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Blend toward the target emotion
	const FBallEyePose Target = EmotionPoses.FindRef(Emotion);
	CurrentPose.EyeScale = FMath::FInterpTo(CurrentPose.EyeScale, Target.EyeScale, DeltaTime, BlendSpeed);
	CurrentPose.UpperLid = FMath::FInterpTo(CurrentPose.UpperLid, Target.UpperLid, DeltaTime, BlendSpeed);
	CurrentPose.UpperLidAngle = FMath::FInterpTo(CurrentPose.UpperLidAngle, Target.UpperLidAngle, DeltaTime, BlendSpeed);
	CurrentPose.LowerLid = FMath::FInterpTo(CurrentPose.LowerLid, Target.LowerLid, DeltaTime, BlendSpeed);
	CurrentPose.bDead = Target.bDead;

	// Blinking (dead balls don't blink)
	if (BlinkTime >= 0.f)
	{
		BlinkTime += DeltaTime;
		if (BlinkTime > BlinkDuration)
		{
			BlinkTime = -1.f;
			TimeToNextBlink = FMath::FRandRange(BlinkInterval.X, BlinkInterval.Y);
		}
	}
	else if (!CurrentPose.bDead)
	{
		TimeToNextBlink -= DeltaTime;
		if (TimeToNextBlink <= 0.f)
		{
			BlinkTime = 0.f;
		}
	}

	PushPoseToMaterial();
}

float UBallFaceComponent::BlinkAmount() const
{
	if (BlinkTime < 0.f)
	{
		return 0.f;
	}
	// Close then open: a triangle curve over the blink duration
	const float T = BlinkTime / BlinkDuration;
	return 1.f - FMath::Abs(T * 2.f - 1.f);
}

void UBallFaceComponent::PushPoseToMaterial()
{
	if (!FaceMaterial)
	{
		return;
	}

	const float Blink = BlinkAmount();
	const float UpperLid = FMath::Min(FMath::Max(CurrentPose.UpperLid, Blink), MaxLidClosure);
	const float LidAngle = FMath::Lerp(CurrentPose.UpperLidAngle, 0.f, Blink);

	FaceMaterial->SetScalarParameterValue(TEXT("EyeScale"), CurrentPose.EyeScale);
	FaceMaterial->SetScalarParameterValue(TEXT("UpperLid"), UpperLid);
	FaceMaterial->SetScalarParameterValue(TEXT("UpperLidAngle"), LidAngle);
	FaceMaterial->SetScalarParameterValue(TEXT("LowerLid"), FMath::Min(CurrentPose.LowerLid, MaxLidClosure));
	FaceMaterial->SetScalarParameterValue(TEXT("Dead"), CurrentPose.bDead ? 1.f : 0.f);
}

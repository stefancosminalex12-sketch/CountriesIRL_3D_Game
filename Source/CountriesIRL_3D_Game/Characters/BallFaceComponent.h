// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallFaceComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;

/** The expressions a ball's eyes can show. */
UENUM(BlueprintType)
enum class EBallEmotion : uint8
{
	Neutral,
	Happy,
	Sad,
	Angry,
	Scared,
	Tired,
	Suspicious,
	Dead,
	Count UMETA(Hidden)
};

/** Shape of both eyes for one emotion. These map 1:1 to parameters of the eye material. */
USTRUCT(BlueprintType)
struct FBallEyePose
{
	GENERATED_BODY()

	/** Overall eye size multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Eyes", meta=(ClampMin=0.3, ClampMax=2.0))
	float EyeScale = 1.f;

	/** How much of the eye the upper lid covers (0 = open, 1 = closed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Eyes", meta=(ClampMin=0.0, ClampMax=1.0))
	float UpperLid = 0.f;

	/** Upper lid tilt in degrees. Positive = inner corners lower (angry), negative = outer corners lower (sad) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Eyes", meta=(ClampMin=-45.0, ClampMax=45.0))
	float UpperLidAngle = 0.f;

	/** How much of the eye the lower lid covers (happy/squinting) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Eyes", meta=(ClampMin=0.0, ClampMax=1.0))
	float LowerLid = 0.f;

	/** Show x_x eyes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Eyes")
	bool bDead = false;
};

/**
 *  Countryball eyes: white, outlined, no mouth. The eyes are drawn by a material (M_BallEyes) on a thin
 *  shell around the ball; this component picks the eye shape for the current emotion, blends between
 *  emotions and blinks. Eye placement/size constants live in the material (Docs/Shaders/BallEyes.hlsl).
 */
UCLASS(ClassGroup=(Ball), meta=(BlueprintSpawnableComponent))
class UBallFaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBallFaceComponent();

	/** Creates the face shell. Must be called from the owning actor's constructor. */
	void CreateFaceMesh(AActor* Owner, USceneComponent* Parent, UStaticMesh* SphereMesh, float BallRadius, float HeightScale = 1.f);

	/** Hides the face for the owning player (used in first-person view) */
	void SetOwnerNoSee(bool bNoSee);

	UFUNCTION(BlueprintCallable, Category="Ball|Face")
	void SetEmotion(EBallEmotion NewEmotion);

	UFUNCTION(BlueprintPure, Category="Ball|Face")
	EBallEmotion GetEmotion() const { return Emotion; }

	/** Shows or hides the eyes (hidden when only bones remain) */
	void SetFaceVisible(bool bVisible);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	virtual void BeginPlay() override;

	/** Eye shape for each emotion */
	UPROPERTY(EditAnywhere, Category="Ball|Face")
	TMap<EBallEmotion, FBallEyePose> EmotionPoses;

	/** How quickly the eyes blend to a new emotion */
	UPROPERTY(EditAnywhere, Category="Ball|Face")
	float BlendSpeed = 10.f;

	/** Seconds between blinks (random within range) */
	UPROPERTY(EditAnywhere, Category="Ball|Face")
	FVector2D BlinkInterval = FVector2D(2.5f, 6.f);

private:

	void PushPoseToMaterial();
	float BlinkAmount() const;

	UPROPERTY() TObjectPtr<UStaticMeshComponent> FaceShell;
	UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> FaceMaterial;

	EBallEmotion Emotion = EBallEmotion::Neutral;

	/** Pose currently shown (blending toward the emotion's pose) */
	FBallEyePose CurrentPose;

	float TimeToNextBlink = 3.f;
	float BlinkTime = -1.f;
};

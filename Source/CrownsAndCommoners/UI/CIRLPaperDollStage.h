// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CIRLPaperDollStage.generated.h"

class ABallCharacter;
class USceneCaptureComponent2D;
class UPointLightComponent;
class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;
class UTexture2D;

/**
 *  A small photo studio far below the world for the Equipment screen: a copy of the player's ball ("doll"),
 *  its own lights and a camera that films it onto a texture with a transparent background.
 *  The studio lights and the doll use lighting channel 1 only, so the doll looks the same by day and by night,
 *  and nothing else in the world is lit by the studio. It keeps running while the game is paused
 *  (the doll blinks and bobs gently in the menu), filming a new picture every frame while the Equipment screen is open.
 */
UCLASS()
class ACIRLPaperDollStage : public AActor
{
	GENERATED_BODY()

public:

	ACIRLPaperDollStage();

	/** The picture of the doll as a UI material (transparent where there's no doll) */
	UMaterialInstanceDynamic* GetPicture() const { return Picture; }

	/** Size of the picture in pixels (square) */
	int32 GetResolution() const { return Resolution; }

	/** Turns the doll (degrees), e.g. from dragging the mouse */
	void AddDollYaw(float Degrees);

	/** Faces the camera again */
	void ResetDollYaw();

	/** Puts a coat of arms on the doll (the player's own, or one being tried on) */
	void SetDollFlag(UTexture2D* Flag);

	/** The doll wears and holds what this inventory does, and keeps up as it changes */
	void MirrorGear(class UCIRLInventoryComponent* Source);

	/** Films only while the Equipment screen shows the picture */
	void SetCapturing(bool bCapture);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category="Paper Doll")
	TObjectPtr<USceneCaptureComponent2D> Capture;

	/** Warm key light from the front left, cool fill from the right, a rim light from behind */
	UPROPERTY(VisibleAnywhere, Category="Paper Doll")
	TObjectPtr<UPointLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, Category="Paper Doll")
	TObjectPtr<UPointLightComponent> FillLight;

	UPROPERTY(VisibleAnywhere, Category="Paper Doll")
	TObjectPtr<UPointLightComponent> RimLight;

	UPROPERTY(Transient)
	TObjectPtr<ABallCharacter> Doll;

	/** Whose gear the doll copies */
	TWeakObjectPtr<class UCIRLInventoryComponent> MirroredGear;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** Shows the render target in the UI with the capture's inverted alpha turned the right way round */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Picture;

	/** Resolution of the picture (square) */
	UPROPERTY(EditAnywhere, Category="Paper Doll")
	int32 Resolution = 1024;

	/** The doll faces the camera turned this much, so it reads as a 3D ball (degrees) */
	UPROPERTY(EditAnywhere, Category="Paper Doll")
	float RestingYaw = -20.f;

private:

	float DollYaw = 0.f;

	/** Scene captures don't update by themselves while the game is paused, so the stage films every frame itself */
	bool bCapturing = false;
	void ApplyDollYaw();
};

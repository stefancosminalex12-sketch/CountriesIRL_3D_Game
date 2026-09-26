// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallCharacter.h"
#include "PlayerBallCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
struct FInputActionValue;

/**
 *  The player's ball: adds cameras (first-person by default, third-person toggle) and input handling.
 */
UCLASS()
class APlayerBallCharacter : public ABallCharacter
{
	GENERATED_BODY()

public:

	APlayerBallCharacter();

	UFUNCTION(BlueprintCallable, Category="Camera")
	void SetFirstPerson(bool bEnable);

	UFUNCTION(BlueprintPure, Category="Camera")
	bool IsFirstPerson() const { return bFirstPerson; }

	/** Console: Emotion Angry (or any EBallEmotion name) */
	UFUNCTION(Exec)
	void Emotion(const FString& Name);

	/** Console: switch between first- and third-person (same as the V key) */
	UFUNCTION(Exec)
	void ToggleCamera() { ToggleView(); }

	/** Console (testing): acts as if movement keys are held. DevWalk 0 1 3 1 = strafe right for 3s while running */
	UFUNCTION(Exec)
	void DevWalk(float Forward, float Right, float Seconds, bool bRun = false);

	/** Console (testing): DevDamage 25 */
	UFUNCTION(Exec)
	void DevDamage(float Amount);

	/** Console (testing): DevHeal 25 */
	UFUNCTION(Exec)
	void DevHeal(float Amount);

	/** Console (testing): throw a punch (same as left click) */
	UFUNCTION(Exec)
	void DevPunch() { Attack(); }

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

	/** The design calls for first-person by default */
	UPROPERTY(EditAnywhere, Category="Camera")
	bool bStartInFirstPerson = true;

	/** In first-person, sideways (A/D) movement is slower than forward, like a real side-step */
	UPROPERTY(EditAnywhere, Category="Ball|Movement", meta=(ClampMin=0.1, ClampMax=1.0))
	float StrafeSpeedScale = 0.7f;

private:

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint() { SetSprinting(true); }
	void StopSprint() { SetSprinting(false); }
	void ToggleView() { SetFirstPerson(!bFirstPerson); }
	void CycleEmotion();
	void Attack();

	bool bFirstPerson = false;

	/** DevWalk state */
	FVector2D DevMoveInput = FVector2D::ZeroVector;
	float DevMoveTimeLeft = 0.f;
};

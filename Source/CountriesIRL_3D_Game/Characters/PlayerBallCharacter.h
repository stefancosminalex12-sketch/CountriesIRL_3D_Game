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

protected:

	virtual void BeginPlay() override;
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

private:

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint() { SetSprinting(true); }
	void StopSprint() { SetSprinting(false); }
	void ToggleView() { SetFirstPerson(!bFirstPerson); }
	void CycleEmotion();

	bool bFirstPerson = false;
};

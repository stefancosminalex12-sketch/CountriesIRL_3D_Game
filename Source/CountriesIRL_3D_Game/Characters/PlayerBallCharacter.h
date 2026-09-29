// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallCharacter.h"
#include "PlayerBallCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
struct FInputActionValue;
enum class EHorseGait : uint8;

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

	/** What pressing Interact would do right now, e.g. "E  Get on the horse" (empty = nothing) */
	FString GetInteractPrompt() const;

	/** Console: Emotion Angry (or any EBallEmotion name) */
	UFUNCTION(Exec)
	void Emotion(const FString& Name);

	/** Console: switch between first- and third-person (same as the V key) */
	UFUNCTION(Exec)
	void ToggleCamera() { ToggleView(); }

	/** Console (testing): acts as if movement keys are held. DevWalk 0 1 3 1 = strafe right for 3s while running */
	UFUNCTION(Exec)
	void DevWalk(float Forward, float Right, float Seconds, bool bRun = false);

	/** Console (testing): point the camera. DevLook -5 90 = nearly level, facing east (for watching the legs) */
	UFUNCTION(Exec)
	void DevLook(float Pitch, float Yaw);

	/** Console (testing): DevDamage 25 */
	UFUNCTION(Exec)
	void DevDamage(float Amount);

	/** Console (testing): DevHeal 25 */
	UFUNCTION(Exec)
	void DevHeal(float Amount);

	/** Console (testing): throw a punch (same as left click) */
	UFUNCTION(Exec)
	void DevPunch() { Attack(); }

	/** Console (testing): press the interact key (get on/off a horse) */
	UFUNCTION(Exec)
	void DevInteract() { Interact(); }

	/** Console (testing): get on the nearest free horse wherever it is */
	UFUNCTION(Exec)
	void DevRide();

	/** Console (testing): press jump (on foot or on horseback) */
	UFUNCTION(Exec)
	void DevJump() { JumpPressed(); }

	/** Console (testing): toggle the guard (same as holding right click) */
	UFUNCTION(Exec)
	void DevGuard();

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

	/** How far the first-person view nudges forward with a punch (cm) */
	UPROPERTY(EditAnywhere, Category="Camera")
	float PunchCameraNudge = 6.f;

	/** How close a horse must be to get on it (cm, center to center) */
	UPROPERTY(EditAnywhere, Category="Riding")
	float MountRange = 260.f;

	/** Third-person camera distance on foot and in the saddle (further back to see the horse) */
	UPROPERTY(EditAnywhere, Category="Camera")
	FVector2D CameraDistance = FVector2D(380.f, 600.f);

	/** In the saddle, a Shift press shorter than this is a tap (walk <-> trot); held longer it canters */
	UPROPERTY(EditAnywhere, Category="Ball|Riding")
	float ShiftTapSeconds = 0.25f;

	/** A second Shift press this soon after a tap is a double press: gallop while held */
	UPROPERTY(EditAnywhere, Category="Ball|Riding")
	float ShiftDoublePressSeconds = 0.3f;

	/** In first-person, sideways (A/D) movement is slower than forward, like a real side-step */
	UPROPERTY(EditAnywhere, Category="Ball|Movement", meta=(ClampMin=0.1, ClampMax=1.0))
	float StrafeSpeedScale = 0.7f;

private:

	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void JumpPressed();
	void JumpReleased();
	void Interact();

	/** Nearest free horse close enough to get on */
	class AHorse* FindHorseToMount() const;
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();

	/** In the saddle: the gait asked for with W and Shift */
	EHorseGait GetRideGait() const;
	void ToggleView() { SetFirstPerson(!bFirstPerson); }
	void CycleEmotion();
	void Attack();
	void StartGuard();
	void StopGuard();
	void UpdateRotationMode();

	bool bFirstPerson = false;

	/** In the saddle: W walks; a tap of Shift switches to a trot (and back). Stopping resets to a walk */
	bool bRideAtTrot = false;

	/** Shift timing in the saddle: when it was pressed, the last tap, and whether this press is a double press */
	float ShiftPressTime = -100.f;
	float LastShiftTapTime = -100.f;
	bool bTrotBeforeTap = false;
	bool bShiftDoublePress = false;

	/** Letting go of Shift at a canter or gallop keeps that pace for a moment, so pressing again
	 *  quickly (to go up to a gallop) doesn't make the horse slow down in between */
	EHorseGait HeldGait{};
	float HeldGaitUntil = -100.f;

	/** Resting place of the first-person camera */
	FVector FirstPersonCameraOffset = FVector::ZeroVector;

	/** DevWalk state */
	FVector2D DevMoveInput = FVector2D::ZeroVector;
	float DevMoveTimeLeft = 0.f;
};

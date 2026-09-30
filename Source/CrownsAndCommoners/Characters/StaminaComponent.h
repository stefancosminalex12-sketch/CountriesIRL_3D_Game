// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

/**
 *  Stamina for anything that can tire: running drains it over time, jumps (and later attacks) cost
 *  a fixed amount. It refills after a short rest. Running to zero leaves you exhausted until it
 *  recovers to a threshold, so you can't spam sprint at empty stamina.
 */
UCLASS(ClassGroup=(Ball), meta=(BlueprintSpawnableComponent))
class UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UStaminaComponent();

	/** Spends a fixed amount (e.g. a jump). Returns false and spends nothing if there isn't enough. */
	bool TryConsume(float Amount);

	/** Drains continuously (e.g. running). Call every frame the activity continues. */
	void Drain(float PerSecond, float DeltaTime);

	/** True if there is at least Amount available and we are not exhausted */
	bool HasStamina(float Amount = 0.f) const;

	UFUNCTION(BlueprintPure, Category="Stamina")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category="Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category="Stamina")
	float GetStaminaPercent() const { return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f; }

	UFUNCTION(BlueprintPure, Category="Stamina")
	bool IsExhausted() const { return bExhausted; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Changes the capacity (e.g. a horse has far more than a person), keeping how full it is */
	void SetMaxStamina(float NewMax);

protected:

	/** Everyone starts rested */
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Stamina", meta=(ClampMin=1))
	float MaxStamina = 100.f;

	/** Stamina regained per second once resting */
	UPROPERTY(EditAnywhere, Category="Stamina", meta=(ClampMin=0))
	float RegenPerSecond = 14.f;

	/** Seconds after last use before stamina starts refilling */
	UPROPERTY(EditAnywhere, Category="Stamina", meta=(ClampMin=0))
	float RegenDelay = 1.2f;

	/** After hitting zero, stamina must refill to this before running/jumping again */
	UPROPERTY(EditAnywhere, Category="Stamina", meta=(ClampMin=0))
	float RecoverThreshold = 30.f;

private:

	void MarkUsed();

	float Stamina = 100.f;
	float TimeSinceUse = 0.f;
	bool bExhausted = false;
};

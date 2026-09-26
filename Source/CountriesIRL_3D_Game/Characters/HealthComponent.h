// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDepleted, UHealthComponent*, Health);

/**
 *  Hit points for anything that can be hurt (balls now; horses and destructible things later).
 *  Base is 100. Reaching zero broadcasts OnDepleted once.
 */
UCLASS(ClassGroup=(Ball), meta=(BlueprintSpawnableComponent))
class UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UHealthComponent();

	/** Removes health; returns how much was actually removed */
	UFUNCTION(BlueprintCallable, Category="Health")
	float ApplyDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category="Health")
	void Heal(float Amount);

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDepleted() const { return Health <= 0.f; }

	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnHealthDepleted OnDepleted;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Health", meta=(ClampMin=1))
	float MaxHealth = 100.f;

private:

	float Health = 100.f;
};

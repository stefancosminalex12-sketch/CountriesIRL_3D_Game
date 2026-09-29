// CountriesIRL 3D Game

#include "Characters/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

float UHealthComponent::ApplyDamage(float Amount)
{
	if (Amount <= 0.f || IsDepleted())
	{
		return 0.f;
	}

	const float Removed = FMath::Min(Amount, Health);
	Health -= Removed;

	if (IsDepleted())
	{
		OnDepleted.Broadcast(this);
	}
	return Removed;
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.f || IsDepleted())
	{
		return;
	}
	Health = FMath::Min(Health + Amount, MaxHealth);
}

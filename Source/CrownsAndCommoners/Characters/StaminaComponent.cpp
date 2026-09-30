// Crowns & Commoners

#include "Characters/StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UStaminaComponent::HasStamina(float Amount) const
{
	return !bExhausted && Stamina >= FMath::Max(Amount, KINDA_SMALL_NUMBER);
}

bool UStaminaComponent::TryConsume(float Amount)
{
	if (!HasStamina(Amount))
	{
		return false;
	}

	Stamina -= Amount;
	MarkUsed();
	return true;
}

void UStaminaComponent::Drain(float PerSecond, float DeltaTime)
{
	if (bExhausted)
	{
		return;
	}

	Stamina = FMath::Max(Stamina - PerSecond * DeltaTime, 0.f);
	MarkUsed();
}

void UStaminaComponent::MarkUsed()
{
	TimeSinceUse = 0.f;
	if (Stamina <= 0.f)
	{
		bExhausted = true;
	}
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceUse += DeltaTime;
	if (TimeSinceUse >= RegenDelay)
	{
		Stamina = FMath::Min(Stamina + RegenPerSecond * DeltaTime, MaxStamina);
	}

	if (bExhausted && Stamina >= RecoverThreshold)
	{
		bExhausted = false;
	}
}

void UStaminaComponent::SetMaxStamina(float NewMax)
{
	const float Percent = GetStaminaPercent();
	MaxStamina = FMath::Max(NewMax, 1.f);
	Stamina = MaxStamina * Percent;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	Stamina = MaxStamina;
	bExhausted = false;
}

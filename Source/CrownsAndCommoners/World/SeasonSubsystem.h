// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SeasonSubsystem.generated.h"

class UClimateProfile;
class UMaterialParameterCollection;

UENUM(BlueprintType)
enum class ESeason : uint8
{
	Spring,
	Summer,
	Autumn,
	Winter
};

/** What nature is like right now */
USTRUCT(BlueprintType)
struct FSeasonState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Season")
	ESeason Season = ESeason::Spring;

	/** Air temperature now (°C): coldest before dawn, warmest mid-afternoon */
	UPROPERTY(BlueprintReadOnly, Category="Season")
	float Temperature = 10.f;

	/** Tree foliage: 0 = bare, 1 = full canopy */
	UPROPERTY(BlueprintReadOnly, Category="Season")
	float LeafAmount = 1.f;

	UPROPERTY(BlueprintReadOnly, Category="Season")
	FLinearColor LeafTint = FLinearColor::Green;

	UPROPERTY(BlueprintReadOnly, Category="Season")
	FLinearColor GrassTint = FLinearColor::Green;

	/** White frost on the ground: 0..1, forms on freezing nights and melts as the day warms */
	UPROPERTY(BlueprintReadOnly, Category="Season")
	float Frost = 0.f;

	/** Morning mist: 0..1, on cool mornings, mostly autumn and winter */
	UPROPERTY(BlueprintReadOnly, Category="Season")
	float Mist = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeasonChanged, ESeason, NewSeason);

/**
 *  Seasons: turns the world clock and the region's climate profile into temperature, foliage,
 *  grass colour, frost and mist. The weather system builds on these and writes everything into the
 *  MPC_Season material parameter collection, so any tree, grass or ground material can follow it.
 */
UCLASS()
class USeasonSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintPure, Category="Season")
	const FSeasonState& GetState() const { return State; }

	static FString SeasonName(ESeason Season);

	/** Nature at an astronomical (Gregorian) date and solar hour, for a climate */
	static FSeasonState Evaluate(const UClimateProfile& Profile, const FDateTime& AstronomicalDate, float SolarHour);

	UPROPERTY(BlueprintAssignable, Category="Season")
	FOnSeasonChanged OnSeasonChanged;

protected:

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:

	/** Advances the lingering frost by some hours at a moment in time */
	void StepFrost(const FDateTime& AstronomicalDate, float Hours);

	UPROPERTY(Transient)
	TObjectPtr<UClimateProfile> Climate;

	FSeasonState State;
	bool bHasState = false;

	/** Frost builds up overnight and melts gradually in the morning, so it needs memory */
	float Frost = 0.f;
	FDateTime LastClockTime;

	/** Frost melted per in-game hour in strong sunshine (less under a low sun, none at night) */
	UPROPERTY(EditAnywhere, Category="Season")
	float FrostMeltPerHour = 0.6f;
};

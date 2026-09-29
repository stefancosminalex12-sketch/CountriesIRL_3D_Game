// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WeatherSubsystem.generated.h"

class UClimateProfile;
class UMaterialParameterCollection;
struct FSeasonState;

/** Weather that can be forced for testing (DevWeather) */
UENUM(BlueprintType)
enum class EForcedWeather : uint8
{
	None,
	Clear,
	Fair,
	Cloudy,
	Overcast,
	Showers,
	Rain,
	HeavyRain,
	Storm,
	Snow,
	Fog
};

/** The weather right now */
USTRUCT(BlueprintType)
struct FWeatherState
{
	GENERATED_BODY()

	/** Share of the sky covered by cloud, 0..1 */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float CloudCover = 0.5f;

	/** How hard it rains or snows, 0 (dry) .. 1 (downpour) */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float Precipitation = 0.f;

	/** Falling as snow (cold enough) instead of rain */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	bool bSnowing = false;

	/** Summer downpour with thunder */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	bool bThunder = false;

	/** Air temperature (degrees C) including clouds, rain and height above sea level */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float Temperature = 10.f;

	/** Wind speed (m/s) and the direction it blows FROM (degrees, 0 = north, 90 = east) */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float WindSpeed = 4.f;

	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float WindFrom = 225.f;

	/** Fog/mist, 0..1 */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float Fog = 0.f;

	/** How wet the ground is (0 dry .. 1 soaked) and how much snow lies on it (0 .. 1 fully white) */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float Wetness = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float SnowCover = 0.f;

	/** Frost on the ground (from the seasons, less under cloud) */
	UPROPERTY(BlueprintReadOnly, Category="Weather")
	float Frost = 0.f;
};

/**
 *  Weather: fronts passing every few days, showers on summer afternoons, fog on still clear mornings,
 *  snow when it is cold enough, the ground getting wet and drying, snow settling and melting.
 *  Everything follows the region's climate profile (how often it rains, how cloudy each month is),
 *  the year (cold summers after 1453) and the height above sea level (colder on the hills).
 *  Like the rest of the world state it is worked out from the date and time, so no save data is needed:
 *  after time jumps the last days are replayed to get wet ground and lying snow right.
 *  Also the single place that writes the environment into MPC_Season for materials.
 */
UCLASS()
class UWeatherSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintPure, Category="Weather")
	const FWeatherState& GetState() const { return State; }

	/** Testing: force a kind of weather (None = back to the natural weather) */
	void ForceWeather(EForcedWeather Weather) { Forced = Weather; bReplayNeeded = true; }

	/** "Overcast, light rain" */
	FString Describe() const;

	/** The sky and precipitation at a moment, before the ground's memory (wetness, snow) is applied */
	static FWeatherState Evaluate(const UClimateProfile& Climate, const FDateTime& AstronomicalDate, float SolarHour,
		const FSeasonState& Season, float AltitudeMeters, EForcedWeather Forced);

protected:

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:

	/** Moves wet ground and lying snow forward by some hours */
	void StepGround(const FWeatherState& Now, float Hours);

	/** Evaluates the weather (with the seasons) at a past or present astronomical moment */
	FWeatherState EvaluateAt(const FDateTime& AstronomicalDate) const;

	/** Height of the viewer above sea level (m) */
	float GetViewerAltitude() const;

	void PushToMaterials() const;

	UPROPERTY(Transient)
	TObjectPtr<UClimateProfile> Climate;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialParameterCollection> Parameters;

	FWeatherState State;
	float Wetness = 0.f;
	float SnowCover = 0.f;
	FDateTime LastClockTime;
	bool bReplayNeeded = true;
	EForcedWeather Forced = EForcedWeather::None;
};

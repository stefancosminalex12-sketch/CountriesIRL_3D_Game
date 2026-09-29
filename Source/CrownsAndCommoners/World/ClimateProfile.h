// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ClimateProfile.generated.h"

/** Typical conditions for one month; the season system blends between neighbouring months */
USTRUCT(BlueprintType)
struct FMonthlyClimate
{
	GENERATED_BODY()

	/** Average air temperature over day and night (°C) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climate")
	float MeanTemperature = 10.f;

	/** Difference between the warmest afternoon and the coldest pre-dawn hour (°C) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climate")
	float DailyRange = 8.f;

	/** How much of the trees' foliage is out: 0 = bare branches, 1 = full canopy */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nature", meta=(ClampMin=0, ClampMax=1))
	float LeafAmount = 1.f;

	/** Colour of broadleaf foliage (fresh green in spring, dark in summer, gold/brown in autumn) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nature")
	FLinearColor LeafTint = FLinearColor(0.1f, 0.25f, 0.05f);

	/** Colour of meadow grass (lush in spring, hay-coloured in late summer, dull in winter) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nature")
	FLinearColor GrassTint = FLinearColor(0.12f, 0.3f, 0.05f);

	/** Days in the month with at least 1 mm of rain or snow */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weather")
	float RainDays = 10.f;

	/** Average share of the sky covered by cloud, 0..1 (from sunshine records) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weather", meta=(ClampMin=0, ClampMax=1))
	float MeanCloudCover = 0.65f;

	/** How much of the rain comes as short showers (sunny spells between) rather than long spells from weather fronts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weather", meta=(ClampMin=0, ClampMax=1))
	float ShowerShare = 0.3f;

	/** Average wind speed (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weather")
	float MeanWind = 4.f;
};

/**
 *  The climate of a region: what each month is like. Values are for the middle of the month, by the
 *  astronomical (Gregorian) calendar, because nature follows the sun, not the calendar on the wall.
 *  A DLC region brings its own profile. Defaults: Yorkshire in the 1450s (about half a degree
 *  cooler than the late 20th century: the Little Ice Age).
 */
UCLASS(BlueprintType)
class UClimateProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UClimateProfile();

	/** January to December */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climate", EditFixedSize)
	TArray<FMonthlyClimate> Months;

	/** How much warmer or colder a single day can be than the monthly average (°C) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climate")
	float DayToDayVariation = 2.5f;

	/**
	 *  Unusual years: extra warmth/cold (degrees C) for a given year, felt fully in summer and half in winter.
	 *  Default: the cold summers after 1453 (a huge volcanic eruption; English oaks grew abnormally narrow rings 1453-1455).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climate")
	TMap<int32, float> YearAnomalies;

	/** Height above sea level where the numbers were measured (m). Higher ground is colder: -0.65 degrees C per 100 m. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climate")
	float ReferenceAltitude = 14.f;

	/** Typical hours of rain on a day that counts as a rain day (British rain days are rarely wet all day) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weather")
	float RainHoursPerRainDay = 5.5f;

	/** Warmer or colder than normal this year (degrees C) for a month */
	float GetYearAnomaly(int32 Year, int32 Month) const;
};

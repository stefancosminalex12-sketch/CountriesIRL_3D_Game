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
};

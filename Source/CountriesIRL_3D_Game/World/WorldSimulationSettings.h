// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "WorldSimulationSettings.generated.h"

/**
 *  Project-wide settings for the living world (Project Settings > Game > CountriesIRL World).
 *  Stored in Config/DefaultGame.ini so DLC regions can override them with data, not code.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="CountriesIRL World"))
class UWorldSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/** In-game date and time when a new game starts. Dates are in the Julian calendar, as used in England until 1752. */
	UPROPERTY(config, EditAnywhere, Category="Time")
	FDateTime StartDateTime = FDateTime(1455, 5, 1, 7, 0, 0);

	/** Real minutes for one full in-game day while playing normally */
	UPROPERTY(config, EditAnywhere, Category="Time", meta=(ClampMin=1))
	float DayLengthMinutes = 48.f;

	/** Days to add to a Julian date to get the astronomical (Gregorian) date. 9 for 1300-1500, 10 for 1500-1700. */
	UPROPERTY(config, EditAnywhere, Category="Time")
	int32 JulianToGregorianDays = 9;

	/** Latitude used for the sun and moon (degrees north). York is 53.96. */
	UPROPERTY(config, EditAnywhere, Category="Sky")
	float Latitude = 53.96f;
};

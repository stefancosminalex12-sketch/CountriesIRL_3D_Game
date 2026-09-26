// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"

/**
 *  Simple astronomy for the sky. World convention: +X is north, +Y is east, +Z is up.
 *  Time is local apparent solar time (what sundials showed; medieval hours followed the sun).
 */
namespace SkyMath
{
	struct FSkyPosition
	{
		double ElevationDeg = 0.0;
		/** Degrees clockwise from north */
		double AzimuthDeg = 0.0;

		/** Unit vector pointing from the ground toward the body */
		FVector ToDirection() const;
	};

	/** Sun position for a latitude, astronomical day of year (1-366) and solar hour (0-24) */
	FSkyPosition SunPosition(double LatitudeDeg, int32 DayOfYear, double SolarHour);

	/** Moon phase 0..1 (0 = new, 0.5 = full) for an astronomical (Gregorian) date */
	double MoonPhase(const FDateTime& GregorianDate);

	/** Approximate moon position: trails the sun by the phase, opposite the sun at full moon */
	FSkyPosition MoonPosition(double LatitudeDeg, int32 DayOfYear, double SolarHour, double Phase);

	/** How much of the moon is lit, 0..1 */
	double MoonIllumination(double Phase);
}

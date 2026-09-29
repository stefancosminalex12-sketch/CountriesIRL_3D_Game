// CountriesIRL 3D Game

#include "World/SkyMath.h"

namespace SkyMath
{
	namespace
	{
		constexpr double SynodicMonthDays = 29.530588853;

		/** Solar declination (radians) from the NOAA series approximation */
		double SolarDeclination(int32 DayOfYear, double SolarHour)
		{
			const double Gamma = UE_DOUBLE_TWO_PI / 365.0 * (DayOfYear - 1 + (SolarHour - 12.0) / 24.0);
			return 0.006918 - 0.399912 * FMath::Cos(Gamma) + 0.070257 * FMath::Sin(Gamma)
				- 0.006758 * FMath::Cos(2.0 * Gamma) + 0.000907 * FMath::Sin(2.0 * Gamma)
				- 0.002697 * FMath::Cos(3.0 * Gamma) + 0.00148 * FMath::Sin(3.0 * Gamma);
		}

		/** Horizontal position from hour angle (degrees, 0 = due south, positive = afternoon) and declination */
		FSkyPosition FromHourAngle(double LatitudeDeg, double HourAngleDeg, double DeclinationRad)
		{
			const double Lat = FMath::DegreesToRadians(LatitudeDeg);
			const double HourAngle = FMath::DegreesToRadians(HourAngleDeg);

			const double SinElevation = FMath::Sin(Lat) * FMath::Sin(DeclinationRad) + FMath::Cos(Lat) * FMath::Cos(DeclinationRad) * FMath::Cos(HourAngle);
			const double Elevation = FMath::Asin(FMath::Clamp(SinElevation, -1.0, 1.0));

			const double Denominator = FMath::Cos(Elevation) * FMath::Cos(Lat);
			const double CosAzimuth = Denominator > UE_DOUBLE_SMALL_NUMBER
				? (FMath::Sin(DeclinationRad) - SinElevation * FMath::Sin(Lat)) / Denominator
				: 1.0;
			double Azimuth = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAzimuth, -1.0, 1.0)));

			// Normalize the hour angle to -180..180 so "afternoon" means west of south
			const double WrappedHourAngle = FMath::UnwindDegrees(HourAngleDeg);
			if (WrappedHourAngle > 0.0)
			{
				Azimuth = 360.0 - Azimuth;
			}

			FSkyPosition Position;
			Position.ElevationDeg = FMath::RadiansToDegrees(Elevation);
			Position.AzimuthDeg = Azimuth;
			return Position;
		}
	}

	FVector FSkyPosition::ToDirection() const
	{
		const double El = FMath::DegreesToRadians(ElevationDeg);
		const double Az = FMath::DegreesToRadians(AzimuthDeg);
		return FVector(FMath::Cos(El) * FMath::Cos(Az), FMath::Cos(El) * FMath::Sin(Az), FMath::Sin(El));
	}

	FSkyPosition SunPosition(double LatitudeDeg, int32 DayOfYear, double SolarHour)
	{
		return FromHourAngle(LatitudeDeg, (SolarHour - 12.0) * 15.0, SolarDeclination(DayOfYear, SolarHour));
	}

	double MoonPhase(const FDateTime& GregorianDate)
	{
		// A known new moon: 6 January 2000, 18:14 UT
		static const FDateTime ReferenceNewMoon(2000, 1, 6, 18, 14, 0);
		const double Cycles = (GregorianDate - ReferenceNewMoon).GetTotalDays() / SynodicMonthDays;
		return Cycles - FMath::Floor(Cycles);
	}

	FSkyPosition MoonPosition(double LatitudeDeg, int32 DayOfYear, double SolarHour, double Phase)
	{
		// The moon rises about 50 minutes later each day: it trails the sun by Phase of a full turn.
		// Its declination is roughly mirrored from the sun's at full moon.
		const double HourAngle = (SolarHour - 12.0) * 15.0 - Phase * 360.0;
		const double Declination = SolarDeclination(DayOfYear, SolarHour) * FMath::Cos(Phase * UE_DOUBLE_TWO_PI);
		return FromHourAngle(LatitudeDeg, HourAngle, Declination);
	}

	double MoonIllumination(double Phase)
	{
		return 0.5 * (1.0 - FMath::Cos(Phase * UE_DOUBLE_TWO_PI));
	}
}

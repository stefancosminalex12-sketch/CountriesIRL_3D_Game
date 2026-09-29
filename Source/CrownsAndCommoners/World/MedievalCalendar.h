// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"

/**
 *  The calendar as 15th-century English people lived it: feast days, church seasons and the work of
 *  the farming year. Dates are Julian (see UWorldClockSubsystem). Used for the HUD now; later for
 *  NPC schedules (no work on feast days, markets, fasting in Lent) and events.
 */
namespace MedievalCalendar
{
	/** Easter Sunday of a year in the Julian calendar (the rule the medieval Church used) */
	FDateTime EasterSunday(int32 Year);

	/** Weekday of a Julian date (weekdays are the same in both calendars) */
	EDayOfWeek DayOfWeek(const FDateTime& JulianDate);

	/** Feast day on this date, e.g. "Lammas", or empty */
	FString FeastDay(const FDateTime& JulianDate);

	/** Church season on this date: Advent, Christmastide, Lent, Eastertide, or empty (ordinary time) */
	FString ChurchSeason(const FDateTime& JulianDate);

	/** Main farm work of the month, e.g. "haymaking" */
	FString FarmWork(const FDateTime& JulianDate);
}

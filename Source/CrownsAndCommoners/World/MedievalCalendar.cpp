// Crowns & Commoners

#include "World/MedievalCalendar.h"
#include "World/WorldSimulationSettings.h"

namespace
{
	FDateTime DateOnly(const FDateTime& Date)
	{
		return Date.GetDate();
	}

	int32 DaysBetween(const FDateTime& From, const FDateTime& To)
	{
		return FMath::RoundToInt((DateOnly(To) - DateOnly(From)).GetTotalDays());
	}

	struct FFixedFeast { int32 Month, Day; const TCHAR* Name; };

	const FFixedFeast FixedFeasts[] =
	{
		{ 1, 1, TEXT("New Year's Day (gifts are given)") },
		{ 1, 6, TEXT("Twelfth Day (Epiphany)") },
		{ 2, 2, TEXT("Candlemas") },
		{ 2, 14, TEXT("St Valentine's Day") },
		{ 3, 25, TEXT("Lady Day (quarter day, rents due; the legal new year)") },
		{ 4, 23, TEXT("St George's Day (patron of England)") },
		{ 5, 1, TEXT("May Day") },
		{ 6, 24, TEXT("Midsummer (quarter day, bonfires)") },
		{ 6, 29, TEXT("St Peter and St Paul") },
		{ 7, 25, TEXT("St James") },
		{ 8, 1, TEXT("Lammas (first loaf of the harvest)") },
		{ 8, 15, TEXT("Assumption of Our Lady") },
		{ 9, 29, TEXT("Michaelmas (quarter day, rents due)") },
		{ 10, 25, TEXT("St Crispin's Day (Agincourt, 1415)") },
		{ 11, 1, TEXT("All Saints") },
		{ 11, 2, TEXT("All Souls") },
		{ 11, 11, TEXT("Martinmas (livestock slaughtered for winter)") },
		{ 12, 25, TEXT("Christmas (quarter day)") },
		{ 12, 26, TEXT("St Stephen's Day") },
		{ 12, 28, TEXT("Childermas (Holy Innocents)") },
	};

	struct FEasterFeast { int32 DaysFromEaster; const TCHAR* Name; };

	const FEasterFeast EasterFeasts[] =
	{
		{ -47, TEXT("Shrove Tuesday") },
		{ -46, TEXT("Ash Wednesday (Lent begins: no meat)") },
		{ -7, TEXT("Palm Sunday") },
		{ -3, TEXT("Maundy Thursday") },
		{ -2, TEXT("Good Friday") },
		{ 0, TEXT("Easter Day") },
		{ 8, TEXT("Hock Monday") },
		{ 39, TEXT("Ascension Day") },
		{ 49, TEXT("Whitsunday") },
		{ 56, TEXT("Trinity Sunday") },
		{ 60, TEXT("Corpus Christi (York mystery plays)") },
	};

	/** First Sunday of Advent: the fourth Sunday before Christmas */
	FDateTime AdventSunday(int32 Year)
	{
		FDateTime Date(Year, 12, 24);
		while (MedievalCalendar::DayOfWeek(Date) != EDayOfWeek::Sunday)
		{
			Date -= FTimespan::FromDays(1);
		}
		return Date - FTimespan::FromDays(21);
	}
}

FDateTime MedievalCalendar::EasterSunday(int32 Year)
{
	// Julian computus (Meeus)
	const int32 A = Year % 4;
	const int32 B = Year % 7;
	const int32 C = Year % 19;
	const int32 D = (19 * C + 15) % 30;
	const int32 E = (2 * A + 4 * B - D + 34) % 7;
	const int32 Month = (D + E + 114) / 31;
	const int32 Day = (D + E + 114) % 31 + 1;
	return FDateTime(Year, Month, Day);
}

EDayOfWeek MedievalCalendar::DayOfWeek(const FDateTime& JulianDate)
{
	const int32 Offset = GetDefault<UWorldSimulationSettings>()->JulianToGregorianDays;
	return (JulianDate + FTimespan::FromDays(Offset)).GetDayOfWeek();
}

FString MedievalCalendar::FeastDay(const FDateTime& JulianDate)
{
	const int32 FromEaster = DaysBetween(EasterSunday(JulianDate.GetYear()), JulianDate);
	for (const FEasterFeast& Feast : EasterFeasts)
	{
		if (Feast.DaysFromEaster == FromEaster)
		{
			return Feast.Name;
		}
	}

	// Plough Monday: the first Monday after Twelfth Day, when ploughing starts again
	if (JulianDate.GetMonth() == 1 && JulianDate.GetDay() >= 7 && JulianDate.GetDay() <= 13 && DayOfWeek(JulianDate) == EDayOfWeek::Monday)
	{
		return TEXT("Plough Monday");
	}

	for (const FFixedFeast& Feast : FixedFeasts)
	{
		if (Feast.Month == JulianDate.GetMonth() && Feast.Day == JulianDate.GetDay())
		{
			return Feast.Name;
		}
	}
	return FString();
}

FString MedievalCalendar::ChurchSeason(const FDateTime& JulianDate)
{
	const int32 Year = JulianDate.GetYear();
	const int32 Month = JulianDate.GetMonth();
	const int32 Day = JulianDate.GetDay();

	if ((Month == 12 && Day >= 25) || (Month == 1 && Day <= 5))
	{
		return TEXT("Christmastide");
	}
	if (DateOnly(JulianDate) >= AdventSunday(Year))
	{
		return TEXT("Advent");
	}

	const int32 FromEaster = DaysBetween(EasterSunday(Year), JulianDate);
	if (FromEaster >= -46 && FromEaster < 0)
	{
		return TEXT("Lent");
	}
	if (FromEaster >= 0 && FromEaster <= 49)
	{
		return TEXT("Eastertide");
	}
	return FString();
}

FString MedievalCalendar::FarmWork(const FDateTime& JulianDate)
{
	// The "labours of the months" as shown in English calendars and books of hours
	static const TCHAR* Labours[12] =
	{
		TEXT("threshing, mending tools"),
		TEXT("ploughing, hedging and ditching"),
		TEXT("sowing barley, oats and peas; pruning"),
		TEXT("harrowing, lambing ends"),
		TEXT("weeding the corn, washing sheep"),
		TEXT("sheep shearing, haymaking begins"),
		TEXT("haymaking"),
		TEXT("grain harvest: reaping and binding sheaves"),
		TEXT("harvest home, gleaning, picking fruit"),
		TEXT("ploughing and sowing winter wheat and rye"),
		TEXT("pigs fattened on acorns, Martinmas slaughter"),
		TEXT("salting meat, Christmas feasting"),
	};
	return Labours[JulianDate.GetMonth() - 1];
}

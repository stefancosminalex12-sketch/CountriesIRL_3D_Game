// CountriesIRL 3D Game

#include "World/SeasonSubsystem.h"
#include "World/ClimateProfile.h"
#include "World/WorldClockSubsystem.h"
#include "World/WorldSimulationSettings.h"
#include "World/SkyMath.h"
#include "Engine/World.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

namespace
{
	/** Repeatable "random" number in -1..1 for a day, so each day has its own warmth */
	float DayNoise(int64 Day)
	{
		uint32 Hash = static_cast<uint32>(Day) * 2654435761u;
		Hash ^= Hash >> 15;
		Hash *= 2246822519u;
		Hash ^= Hash >> 13;
		return (Hash & 0xFFFF) / 32767.5f - 1.f;
	}

	ESeason SeasonOfMonth(int32 Month)
	{
		if (Month >= 3 && Month <= 5) { return ESeason::Spring; }
		if (Month >= 6 && Month <= 8) { return ESeason::Summer; }
		if (Month >= 9 && Month <= 11) { return ESeason::Autumn; }
		return ESeason::Winter;
	}
}

void USeasonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UWorldClockSubsystem>();
	Super::Initialize(Collection);

	const UWorldSimulationSettings* Settings = GetDefault<UWorldSimulationSettings>();
	Climate = Settings->Climate.LoadSynchronous();
	if (!Climate)
	{
		// Built-in Yorkshire defaults
		Climate = GetMutableDefault<UClimateProfile>();
	}
	Parameters = Settings->SeasonParameters.LoadSynchronous();
}

bool USeasonSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId USeasonSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USeasonSubsystem, STATGROUP_Tickables);
}

void USeasonSubsystem::Tick(float DeltaTime)
{
	const UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>();
	if (!Clock || !Climate)
	{
		return;
	}

	const ESeason Previous = State.Season;
	State = Evaluate(*Climate, Clock->GetAstronomicalDateTime(), Clock->GetTimeOfDay());

	// Frost forms as fast as the cold allows, but only melts once the sun is on it.
	// Big time jumps (start, waiting, sleeping) replay the last night so the morning is right.
	const double HoursPassed = (Clock->GetDateTime() - LastClockTime).GetTotalHours();
	LastClockTime = Clock->GetDateTime();
	const FDateTime Astronomical = Clock->GetAstronomicalDateTime();
	if (!bHasState || HoursPassed < 0.0 || HoursPassed > 3.0)
	{
		Frost = 0.f;
		for (float HoursAgo = 12.f; HoursAgo >= 0.f; HoursAgo -= 0.5f)
		{
			StepFrost(Astronomical - FTimespan::FromHours(HoursAgo), 0.5f);
		}
	}
	else
	{
		StepFrost(Astronomical, static_cast<float>(HoursPassed));
	}
	State.Frost = Frost;

	PushToMaterials();

	if (bHasState && State.Season != Previous)
	{
		OnSeasonChanged.Broadcast(State.Season);
	}
	bHasState = true;
}

FSeasonState USeasonSubsystem::Evaluate(const UClimateProfile& Profile, const FDateTime& AstronomicalDate, float SolarHour)
{
	FSeasonState Result;
	Result.Season = SeasonOfMonth(AstronomicalDate.GetMonth());
	if (Profile.Months.Num() != 12)
	{
		return Result;
	}

	// Months are defined at their middle: blend between the two nearest ones
	const float YearFraction = (AstronomicalDate.GetDayOfYear() - 1 + SolarHour / 24.f) / 365.25f;
	const float MonthPosition = YearFraction * 12.f - 0.5f;
	const int32 Lower = FMath::FloorToInt(MonthPosition);
	const float Blend = MonthPosition - Lower;
	const FMonthlyClimate& A = Profile.Months[(Lower + 12) % 12];
	const FMonthlyClimate& B = Profile.Months[(Lower + 13) % 12];

	Result.LeafAmount = FMath::Lerp(A.LeafAmount, B.LeafAmount, Blend);
	Result.LeafTint = FMath::Lerp(A.LeafTint, B.LeafTint, Blend);
	Result.GrassTint = FMath::Lerp(A.GrassTint, B.GrassTint, Blend);

	// Temperature: monthly mean, plus a warmer or colder spell for the day (blended smoothly into
	// the next day), plus the daily swing (coldest around 3:00, warmest around 15:00)
	const int64 Day = AstronomicalDate.GetDate().GetTicks() / ETimespan::TicksPerDay;
	const float DayBlend = FMath::SmoothStep(0.f, 1.f, SolarHour / 24.f);
	const float DayOffset = FMath::Lerp(DayNoise(Day), DayNoise(Day + 1), DayBlend) * Profile.DayToDayVariation;
	const float Mean = FMath::Lerp(A.MeanTemperature, B.MeanTemperature, Blend);
	const float Range = FMath::Lerp(A.DailyRange, B.DailyRange, Blend);
	const float Swing = FMath::Cos(2.f * PI * (SolarHour - 15.f) / 24.f) * Range * 0.5f;
	Result.Temperature = Mean + DayOffset + Swing;

	// Ground frost: grass on a still night is a few degrees colder than the air, so it forms
	// with the air still just above freezing (melting speed is handled in Tick)
	Result.Frost = 1.f - FMath::SmoothStep(-1.f, 2.5f, Result.Temperature);

	// Mist hangs on cool mornings, strongest around 7:00
	const float Morning = FMath::Exp(-FMath::Square(SolarHour - 7.f) / (2.f * FMath::Square(2.5f)));
	Result.Mist = (1.f - FMath::SmoothStep(2.f, 10.f, Result.Temperature)) * Morning;

	return Result;
}

void USeasonSubsystem::StepFrost(const FDateTime& AstronomicalDate, float Hours)
{
	const float SolarHour = static_cast<float>(AstronomicalDate.GetTimeOfDay().GetTotalHours());
	const float Target = Evaluate(*Climate, AstronomicalDate, SolarHour).Frost;
	if (Target >= Frost)
	{
		Frost = Target;
		return;
	}

	// Sunshine melts it: slowly under a low winter sun, quickly under a high one
	const double Latitude = GetDefault<UWorldSimulationSettings>()->Latitude;
	const float SunElevation = static_cast<float>(SkyMath::SunPosition(Latitude, AstronomicalDate.GetDayOfYear(), SolarHour).ElevationDeg);
	const float Melt = FrostMeltPerHour * FMath::SmoothStep(0.f, 20.f, SunElevation) * Hours;
	Frost = FMath::Max(Target, Frost - Melt);
}

void USeasonSubsystem::PushToMaterials() const
{
	UMaterialParameterCollectionInstance* Instance = Parameters ? GetWorld()->GetParameterCollectionInstance(Parameters) : nullptr;
	if (!Instance)
	{
		return;
	}

	Instance->SetScalarParameterValue(TEXT("LeafAmount"), State.LeafAmount);
	Instance->SetScalarParameterValue(TEXT("Frost"), State.Frost);
	Instance->SetScalarParameterValue(TEXT("Mist"), State.Mist);
	Instance->SetScalarParameterValue(TEXT("Temperature"), State.Temperature);
	Instance->SetVectorParameterValue(TEXT("LeafTint"), State.LeafTint);
	Instance->SetVectorParameterValue(TEXT("GrassTint"), State.GrassTint);
}

FString USeasonSubsystem::SeasonName(ESeason Season)
{
	switch (Season)
	{
	case ESeason::Spring: return TEXT("Spring");
	case ESeason::Summer: return TEXT("Summer");
	case ESeason::Autumn: return TEXT("Autumn");
	default: return TEXT("Winter");
	}
}

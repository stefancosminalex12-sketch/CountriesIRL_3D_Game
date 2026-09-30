// Crowns & Commoners

#include "World/WeatherSubsystem.h"
#include "World/ClimateProfile.h"
#include "World/SeasonSubsystem.h"
#include "World/WorldClockSubsystem.h"
#include "World/WorldSimulationSettings.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

namespace
{
	float Hash01(int64 Index)
	{
		uint64 Hash = static_cast<uint64>(Index) * 0x9E3779B97F4A7C15ull;
		Hash ^= Hash >> 31;
		Hash *= 0xBF58476D1CE4E5B9ull;
		Hash ^= Hash >> 29;
		return static_cast<float>(Hash & 0xFFFFFF) / 16777215.f;
	}

	/** Smooth random value 0..1 that changes over X (one new random value per whole unit) */
	float ValueNoise(double X)
	{
		const double Floor = FMath::FloorToDouble(X);
		const float T = static_cast<float>(X - Floor);
		const int64 Index = static_cast<int64>(Floor);
		return FMath::Lerp(Hash01(Index), Hash01(Index + 1), T * T * (3.f - 2.f * T));
	}

	// How high the noise must be to be exceeded a given share of the time. Measured once (2M samples)
	// for exactly these noise mixes, so rain happens as often as the climate data says.
	const float ExceedShare[] = { 0.f, 0.01f, 0.02f, 0.03f, 0.05f, 0.075f, 0.1f, 0.15f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.f };
	const float FrontLevel[] = { 0.9899f, 0.8845f, 0.853f, 0.8306f, 0.7967f, 0.7648f, 0.7383f, 0.6961f, 0.6611f, 0.6019f, 0.549f, 0.4994f, 0.4496f, 0.3975f, 0.3383f, 0.261f, 0.004f };
	const float ShowerLevel[] = { 1.f, 0.9669f, 0.9473f, 0.9303f, 0.9021f, 0.8708f, 0.8429f, 0.7927f, 0.7462f, 0.6602f, 0.5794f, 0.5011f, 0.4237f, 0.3426f, 0.2553f, 0.1588f, 0.0001f };

	float LevelExceeded(const float* Levels, float Share)
	{
		Share = FMath::Clamp(Share, 0.f, 1.f);
		for (int32 Index = 1; Index < UE_ARRAY_COUNT(ExceedShare); ++Index)
		{
			if (Share <= ExceedShare[Index])
			{
				const float Alpha = (Share - ExceedShare[Index - 1]) / (ExceedShare[Index] - ExceedShare[Index - 1]);
				return FMath::Lerp(Levels[Index - 1], Levels[Index], Alpha);
			}
		}
		return Levels[UE_ARRAY_COUNT(ExceedShare) - 1];
	}

	/** The reverse: how much of the time the noise is above this value */
	float ShareAbove(const float* Levels, float Value)
	{
		for (int32 Index = 1; Index < UE_ARRAY_COUNT(ExceedShare); ++Index)
		{
			if (Value >= Levels[Index])
			{
				const float Span = Levels[Index - 1] - Levels[Index];
				const float Alpha = Span > 0.f ? (Levels[Index - 1] - Value) / Span : 0.f;
				return FMath::Lerp(ExceedShare[Index - 1], ExceedShare[Index], Alpha);
			}
		}
		return 1.f;
	}

	/** 1 around midday, 0 at night: how much the sun warms and dries */
	float Daylight(float SolarHour)
	{
		return FMath::Max(FMath::Sin(UE_PI * (SolarHour - 6.f) / 12.f), 0.f);
	}
}

void UWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UWorldClockSubsystem>();
	Collection.InitializeDependency<USeasonSubsystem>();
	Super::Initialize(Collection);

	const UWorldSimulationSettings* Settings = GetDefault<UWorldSimulationSettings>();
	Climate = Settings->Climate.LoadSynchronous();
	if (!Climate)
	{
		Climate = GetMutableDefault<UClimateProfile>();
	}
	Parameters = Settings->SeasonParameters.LoadSynchronous();
}

bool UWeatherSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UWeatherSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWeatherSubsystem, STATGROUP_Tickables);
}

FWeatherState UWeatherSubsystem::Evaluate(const UClimateProfile& Profile, const FDateTime& AstronomicalDate, float SolarHour,
	const FSeasonState& Season, float AltitudeMeters, EForcedWeather Forced)
{
	FWeatherState Result;
	if (Profile.Months.Num() != 12)
	{
		return Result;
	}

	// This time of year: blend the two nearest months (defined at mid-month)
	const float MonthPosition = (AstronomicalDate.GetDayOfYear() - 1 + SolarHour / 24.f) / 365.25f * 12.f - 0.5f;
	const int32 Lower = FMath::FloorToInt(MonthPosition);
	const float Blend = MonthPosition - Lower;
	const FMonthlyClimate& A = Profile.Months[(Lower + 12) % 12];
	const FMonthlyClimate& B = Profile.Months[(Lower + 13) % 12];
	const float RainDays = FMath::Lerp(A.RainDays, B.RainDays, Blend);
	const float MeanCloud = FMath::Lerp(A.MeanCloudCover, B.MeanCloudCover, Blend);
	const float ShowerShare = FMath::Lerp(A.ShowerShare, B.ShowerShare, Blend);
	const float MeanWind = FMath::Lerp(A.MeanWind, B.MeanWind, Blend);

	// Share of all hours with rain, split between long spells from fronts and short daytime showers
	const float RainShare = RainDays / 30.44f * Profile.RainHoursPerRainDay / 24.f;
	const float FrontShare = RainShare * (1.f - ShowerShare);
	// Showers only come between late morning and evening (about 10 of 24 hours)
	const float ShowerWindow = FMath::SmoothStep(9.f, 12.f, SolarHour) * (1.f - FMath::SmoothStep(18.f, 21.f, SolarHour));
	const float ShowerShareWhenPossible = FMath::Min(RainShare * ShowerShare * 24.f / 10.f, 1.f);

	// Weather fronts: changes over a day or two, with faster wobble (British rain comes in many shortish spells)
	const double Hours = static_cast<double>(AstronomicalDate.GetTicks()) / static_cast<double>(ETimespan::TicksPerHour);
	const float Front = 0.6f * ValueNoise(Hours / 14.0) + 0.4f * ValueNoise(Hours / 4.0 + 57.3);
	const float FrontThreshold = LevelExceeded(FrontLevel, FrontShare);
	const float FrontRain = Front > FrontThreshold ? 0.25f + 0.75f * FMath::Clamp((Front - FrontThreshold) / 0.1f, 0.f, 1.f) : 0.f;

	// Showers: a separate quick pattern (an hour or two each), strongest in the afternoon
	const float ShowerNoise = ValueNoise(Hours / 2.5 + 911.7);
	const float ShowerThreshold = LevelExceeded(ShowerLevel, ShowerShareWhenPossible);
	const float ShowerRain = (ShowerWindow > 0.5f && ShowerNoise > ShowerThreshold)
		? 0.4f + 0.6f * FMath::Clamp((ShowerNoise - ShowerThreshold) / 0.05f, 0.f, 1.f) : 0.f;

	Result.Precipitation = FMath::Max(FrontRain, ShowerRain);
	const bool bShower = ShowerRain > FrontRain;

	// Clouds follow the fronts, spread so that the month averages its usual cloudiness; showers come from heaped clouds
	const float FrontRank = 1.f - ShareAbove(FrontLevel, Front);
	Result.CloudCover = FMath::Clamp(0.5f + (FrontRank - (1.f - MeanCloud)) * 2.f, 0.f, 1.f);
	if (FrontRain > 0.f)
	{
		Result.CloudCover = FMath::Max(Result.CloudCover, 0.95f);
	}
	if (ShowerRain > 0.f)
	{
		Result.CloudCover = FMath::Max(Result.CloudCover, 0.75f);
	}

	// Wind: stronger as fronts pass; mostly from the south-west (westerlies), swinging around
	Result.WindSpeed = MeanWind * (0.55f + 1.1f * Front) + (bShower ? 2.f : 0.f);
	Result.WindFrom = FMath::Fmod(225.f + 80.f * (ValueNoise(Hours / 40.0 + 333.1) * 2.f - 1.f) + 360.f, 360.f);

	// Testing: forced weather replaces the sky and precipitation (temperature still follows)
	switch (Forced)
	{
	case EForcedWeather::Clear:     Result.CloudCover = 0.05f; Result.Precipitation = 0.f; break;
	case EForcedWeather::Fair:      Result.CloudCover = 0.3f;  Result.Precipitation = 0.f; break;
	case EForcedWeather::Cloudy:    Result.CloudCover = 0.65f; Result.Precipitation = 0.f; break;
	case EForcedWeather::Overcast:  Result.CloudCover = 0.97f; Result.Precipitation = 0.f; break;
	case EForcedWeather::Showers:   Result.CloudCover = 0.75f; Result.Precipitation = 0.55f; break;
	case EForcedWeather::Rain:      Result.CloudCover = 1.f;   Result.Precipitation = 0.45f; break;
	case EForcedWeather::HeavyRain: Result.CloudCover = 1.f;   Result.Precipitation = 0.9f; break;
	case EForcedWeather::Storm:     Result.CloudCover = 1.f;   Result.Precipitation = 1.f; Result.WindSpeed = 13.f; break;
	case EForcedWeather::Snow:      Result.CloudCover = 1.f;   Result.Precipitation = 0.55f; break;
	case EForcedWeather::Fog:       Result.CloudCover = 0.4f;  Result.Precipitation = 0.f; Result.WindSpeed = 0.5f; break;
	default: break;
	}

	// Temperature: sunny days warm up more, cloudy nights stay milder, rain cools, hills are colder
	const float Day = Daylight(SolarHour);
	float Temperature = Season.Temperature;
	Temperature += (0.45f - Result.CloudCover) * 3.f * Day;
	Temperature += (Result.CloudCover - 0.45f) * 2.f * (1.f - Day);
	Temperature -= Result.Precipitation * 1.5f;
	Temperature -= 0.0065f * (AltitudeMeters - Profile.ReferenceAltitude);
	Result.Temperature = Temperature;

	// Rain turns to snow a little above freezing
	Result.bSnowing = Result.Precipitation > 0.f && (Temperature < 1.5f || Forced == EForcedWeather::Snow);
	Result.bThunder = (bShower && Result.Precipitation > 0.8f && Temperature > 14.f) || Forced == EForcedWeather::Storm;

	// Mist forms on still, clear, cool mornings; clouds and wind stop it
	const float Calm = FMath::Clamp(1.f - (Result.WindSpeed - 2.f) / 5.f, 0.f, 1.f);
	Result.Fog = Season.Mist * (1.f - 0.8f * Result.CloudCover) * Calm;
	if (Forced == EForcedWeather::Fog)
	{
		Result.Fog = 1.f;
	}
	return Result;
}

FWeatherState UWeatherSubsystem::EvaluateAt(const FDateTime& AstronomicalDate) const
{
	const float SolarHour = static_cast<float>(AstronomicalDate.GetTimeOfDay().GetTotalHours());
	const FSeasonState Season = USeasonSubsystem::Evaluate(*Climate, AstronomicalDate, SolarHour);
	return Evaluate(*Climate, AstronomicalDate, SolarHour, Season, GetViewerAltitude(), Forced);
}

void UWeatherSubsystem::StepGround(const FWeatherState& Now, float Hours)
{
	const float Warm = FMath::Max(Now.Temperature, 0.f);

	if (Now.Precipitation > 0.f && !Now.bSnowing)
	{
		// Soaked after about an hour of steady rain
		Wetness += Now.Precipitation * 1.5f * Hours;
	}
	else
	{
		// Dries over half a day to a day: faster when warm, sunny and windy
		const float DryRate = 0.04f + 0.004f * Warm + 0.01f * Now.WindSpeed + 0.05f * (1.f - Now.CloudCover);
		Wetness -= DryRate * Hours;
	}

	if (Now.bSnowing)
	{
		// A couple of hours of steady snow turns the ground white
		SnowCover += Now.Precipitation * 0.6f * Hours;
	}
	else if (Now.Temperature > 0.5f && SnowCover > 0.f)
	{
		// Thaw: warmth, rain and sunshine melt it, and the ground gets wet
		const float Melt = 0.025f * (Now.Temperature - 0.5f) + 0.2f * Now.Precipitation + 0.03f * (1.f - Now.CloudCover);
		SnowCover -= Melt * Hours;
		Wetness = FMath::Max(Wetness, 0.6f);
	}

	Wetness = FMath::Clamp(Wetness, 0.f, 1.f);
	SnowCover = FMath::Clamp(SnowCover, 0.f, 1.f);
}

float UWeatherSubsystem::GetViewerAltitude() const
{
	float Z = 0.f;
	if (const APlayerController* Player = GetWorld()->GetFirstPlayerController())
	{
		if (Player->PlayerCameraManager)
		{
			Z = Player->PlayerCameraManager->GetCameraLocation().Z;
		}
	}
	return (Z - GetDefault<UWorldSimulationSettings>()->WorldZAtSeaLevel) / 100.f;
}

void UWeatherSubsystem::Tick(float DeltaTime)
{
	const UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>();
	const USeasonSubsystem* Seasons = GetWorld()->GetSubsystem<USeasonSubsystem>();
	if (!Clock || !Seasons || !Climate)
	{
		return;
	}

	const FDateTime Astronomical = Clock->GetAstronomicalDateTime();
	const float SolarHour = static_cast<float>(Astronomical.GetTimeOfDay().GetTotalHours());
	State = Evaluate(*Climate, Astronomical, SolarHour, Seasons->GetState(), GetViewerAltitude(), Forced);

	// The ground remembers: wet after rain, white after snow. Time jumps replay the last week hour by hour.
	const double HoursPassed = (Clock->GetDateTime() - LastClockTime).GetTotalHours();
	LastClockTime = Clock->GetDateTime();
	if (bReplayNeeded || HoursPassed < 0.0 || HoursPassed > 3.0)
	{
		Wetness = 0.f;
		SnowCover = 0.f;
		for (int32 HoursAgo = 168; HoursAgo > 0; --HoursAgo)
		{
			StepGround(EvaluateAt(Astronomical - FTimespan::FromHours(HoursAgo)), 1.f);
		}
		bReplayNeeded = false;
	}
	StepGround(State, static_cast<float>(FMath::Clamp(HoursPassed, 0.0, 3.0)));

	State.Wetness = Wetness;
	State.SnowCover = SnowCover;
	// Frost forms under clear skies, and lying snow hides it
	State.Frost = Seasons->GetState().Frost * (1.f - 0.6f * State.CloudCover) * (1.f - SnowCover);

	PushToMaterials();
}

void UWeatherSubsystem::PushToMaterials() const
{
	UMaterialParameterCollectionInstance* Instance = Parameters ? GetWorld()->GetParameterCollectionInstance(Parameters) : nullptr;
	const USeasonSubsystem* Seasons = GetWorld()->GetSubsystem<USeasonSubsystem>();
	if (!Instance || !Seasons)
	{
		return;
	}

	// Nature (seasons)
	const FSeasonState& Season = Seasons->GetState();
	Instance->SetScalarParameterValue(TEXT("LeafAmount"), Season.LeafAmount);
	Instance->SetVectorParameterValue(TEXT("LeafTint"), Season.LeafTint);
	Instance->SetVectorParameterValue(TEXT("GrassTint"), Season.GrassTint);

	// Weather
	Instance->SetScalarParameterValue(TEXT("Temperature"), State.Temperature);
	Instance->SetScalarParameterValue(TEXT("Frost"), State.Frost);
	Instance->SetScalarParameterValue(TEXT("Mist"), State.Fog);
	Instance->SetScalarParameterValue(TEXT("CloudCover"), State.CloudCover);
	Instance->SetScalarParameterValue(TEXT("Rain"), State.bSnowing ? 0.f : State.Precipitation);
	Instance->SetScalarParameterValue(TEXT("Snowfall"), State.bSnowing ? State.Precipitation : 0.f);
	Instance->SetScalarParameterValue(TEXT("Wetness"), State.Wetness);
	Instance->SetScalarParameterValue(TEXT("SnowCover"), State.SnowCover);
	Instance->SetScalarParameterValue(TEXT("Wind"), State.WindSpeed);
}

FString UWeatherSubsystem::Describe() const
{
	FString Sky = State.CloudCover < 0.2f ? TEXT("Clear") : State.CloudCover < 0.45f ? TEXT("Fair")
		: State.CloudCover < 0.8f ? TEXT("Cloudy") : TEXT("Overcast");
	if (State.Fog > 0.5f)
	{
		Sky = TEXT("Foggy");
	}

	FString Falling;
	if (State.bThunder)
	{
		Falling = TEXT("thunderstorm");
	}
	else if (State.Precipitation > 0.f)
	{
		const TCHAR* Amount = State.Precipitation < 0.35f ? TEXT("light ") : State.Precipitation > 0.75f ? TEXT("heavy ") : TEXT("");
		Falling = FString(Amount) + (State.bSnowing ? TEXT("snow") : TEXT("rain"));
	}

	FString Ground;
	if (State.SnowCover > 0.3f)
	{
		Ground = TEXT("snow on the ground");
	}
	else if (State.Wetness > 0.5f && State.Precipitation <= 0.f)
	{
		Ground = TEXT("wet ground");
	}

	FString Text = Sky;
	for (const FString& Part : { Falling, Ground })
	{
		if (!Part.IsEmpty())
		{
			Text += TEXT(", ") + Part;
		}
	}
	static const TCHAR* Compass[] = { TEXT("N"), TEXT("NE"), TEXT("E"), TEXT("SE"), TEXT("S"), TEXT("SW"), TEXT("W"), TEXT("NW") };
	const int32 Sector = FMath::RoundToInt(State.WindFrom / 45.f) % 8;
	return Text + FString::Printf(TEXT(", wind %s %.0f m/s"), Compass[Sector], State.WindSpeed);
}

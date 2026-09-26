// CountriesIRL 3D Game

#include "World/WorldClockSubsystem.h"
#include "World/WorldSimulationSettings.h"
#include "Engine/World.h"

void UWorldClockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UWorldSimulationSettings* Settings = GetDefault<UWorldSimulationSettings>();
	Now = Settings->StartDateTime;
	TimeScale = (24.0 * 60.0) / FMath::Max(Settings->DayLengthMinutes, 1.f);
	JulianToGregorianDays = Settings->JulianToGregorianDays;
}

bool UWorldClockSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// Only in worlds that actually play (game and PIE), not editor preview worlds
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UWorldClockSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWorldClockSubsystem, STATGROUP_Tickables);
}

void UWorldClockSubsystem::Tick(float DeltaTime)
{
	const FDateTime Previous = Now;
	Now += FTimespan::FromSeconds(DeltaTime * TimeScale * SpeedMultiplier);
	BroadcastChanges(Previous);
}

FDateTime UWorldClockSubsystem::GetAstronomicalDateTime() const
{
	return Now + FTimespan::FromDays(JulianToGregorianDays);
}

float UWorldClockSubsystem::GetTimeOfDay() const
{
	return static_cast<float>(Now.GetTimeOfDay().GetTotalHours());
}

void UWorldClockSubsystem::SetTimeOfDay(float Hours)
{
	const FDateTime Previous = Now;
	Now = Now.GetDate() + FTimespan::FromHours(FMath::Clamp(Hours, 0.f, 23.999f));
	BroadcastChanges(Previous);
}

void UWorldClockSubsystem::AdvanceTime(FTimespan Duration)
{
	const FDateTime Previous = Now;
	Now += Duration;
	BroadcastChanges(Previous);
}

void UWorldClockSubsystem::BroadcastChanges(const FDateTime& Previous)
{
	if (Previous.GetDate() != Now.GetDate())
	{
		OnDayChanged.Broadcast(Now.GetDate());
	}
	if (Previous.GetHour() != Now.GetHour() || Previous.GetDate() != Now.GetDate())
	{
		OnHourChanged.Broadcast(Now.GetHour());
	}
}

FString UWorldClockSubsystem::FormatDate() const
{
	static const TCHAR* Weekdays[] = { TEXT("Monday"), TEXT("Tuesday"), TEXT("Wednesday"), TEXT("Thursday"), TEXT("Friday"), TEXT("Saturday"), TEXT("Sunday") };
	static const TCHAR* Months[] = { TEXT("January"), TEXT("February"), TEXT("March"), TEXT("April"), TEXT("May"), TEXT("June"),
		TEXT("July"), TEXT("August"), TEXT("September"), TEXT("October"), TEXT("November"), TEXT("December") };

	// Weekdays run on regardless of calendar, so take them from the astronomical date
	const int32 Weekday = static_cast<int32>(GetAstronomicalDateTime().GetDayOfWeek());
	return FString::Printf(TEXT("%s, %d %s %d"), Weekdays[Weekday], Now.GetDay(), Months[Now.GetMonth() - 1], Now.GetYear());
}

FString UWorldClockSubsystem::FormatTime() const
{
	return FString::Printf(TEXT("%02d:%02d"), Now.GetHour(), Now.GetMinute());
}

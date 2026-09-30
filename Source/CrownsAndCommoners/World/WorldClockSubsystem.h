// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldClockSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClockHourChanged, int32, Hour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClockDayChanged, FDateTime, NewDate);

/**
 *  The in-game calendar and clock. One source of truth for "what time is it" that the sky, weather,
 *  NPC schedules and quests all read from.
 *
 *  The date is kept in the Julian calendar (what people in 15th-century England used); astronomy
 *  (sun, moon, weekdays) uses the equivalent Gregorian date.
 */
UCLASS()
class UWorldClockSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Current date and time (Julian calendar) */
	UFUNCTION(BlueprintPure, Category="Clock")
	FDateTime GetDateTime() const { return Now; }

	/** Same moment in the Gregorian calendar, for astronomy */
	FDateTime GetAstronomicalDateTime() const;

	/** Hours since midnight, 0..24 */
	UFUNCTION(BlueprintPure, Category="Clock")
	float GetTimeOfDay() const;

	/** Jumps to a time of day on the current date (e.g. 21.5 = 21:30) */
	UFUNCTION(BlueprintCallable, Category="Clock")
	void SetTimeOfDay(float Hours);

	/** Jumps to a date (Julian), keeping the time of day. Returns false if the date doesn't exist. */
	UFUNCTION(BlueprintCallable, Category="Clock")
	bool SetDate(int32 Year, int32 Month, int32 Day);

	/** Moves time forward (waiting, sleeping, travel, story jumps) */
	UFUNCTION(BlueprintCallable, Category="Clock")
	void AdvanceTime(FTimespan Duration);

	/** Extra speed on top of normal time (1 = normal 48-minute days). For waiting/sleeping and testing. */
	UFUNCTION(BlueprintCallable, Category="Clock")
	void SetSpeedMultiplier(float Multiplier) { SpeedMultiplier = FMath::Max(Multiplier, 0.f); }

	/** "Thursday, 22 May 1455" */
	FString FormatDate() const;

	/** "07:05" */
	FString FormatTime() const;

	UPROPERTY(BlueprintAssignable, Category="Clock")
	FOnClockHourChanged OnHourChanged;

	UPROPERTY(BlueprintAssignable, Category="Clock")
	FOnClockDayChanged OnDayChanged;

protected:

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:

	void BroadcastChanges(const FDateTime& Previous);

	FDateTime Now;

	/** In-game seconds per real second at normal speed */
	double TimeScale = 30.0;

	float SpeedMultiplier = 1.f;

	int32 JulianToGregorianDays = 9;
};

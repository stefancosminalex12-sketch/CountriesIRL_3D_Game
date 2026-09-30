// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DayNightSky.generated.h"

class UDirectionalLightComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;
class UExponentialHeightFogComponent;
class UPostProcessComponent;

/**
 *  The whole sky in one actor: sun, moon, atmosphere, clouds, sky light, fog and exposure.
 *  Place one per level. During play it follows the world clock (real sun/moon positions for the
 *  latitude and date); in the editor it shows PreviewHour.
 *  World convention: +X is north.
 */
UCLASS()
class ADayNightSky : public AActor
{
	GENERATED_BODY()

public:

	ADayNightSky();

	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDirectionalLightComponent> Sun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UDirectionalLightComponent> Moon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USkyAtmosphereComponent> Atmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UVolumetricCloudComponent> Clouds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UExponentialHeightFogComponent> Fog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPostProcessComponent> Exposure;

	/** Falling rain and snow around the viewer */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<class UPrecipitationComponent> Rain;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<class UPrecipitationComponent> Snow;

	/** Time of day shown in the editor (hours) */
	UPROPERTY(EditAnywhere, Category="Sky", meta=(ClampMin=0, ClampMax=24))
	float PreviewHour = 10.f;

	/** Sun brightness at full daylight (lux) */
	UPROPERTY(EditAnywhere, Category="Sky")
	float SunIntensity = 3.5f;

	/** Moon brightness at full moon (lux) */
	UPROPERTY(EditAnywhere, Category="Sky")
	float MoonIntensity = 0.12f;

	/** Moonlight never drops below this fraction of full, so new-moon nights stay playable (starlight) */
	UPROPERTY(EditAnywhere, Category="Sky", meta=(ClampMin=0, ClampMax=1))
	float MinMoonlight = 0.2f;

	/** Camera exposure in daylight and at night (EV100). Lower = brighter image. */
	UPROPERTY(EditAnywhere, Category="Sky")
	float DayExposure = 0.f;

	UPROPERTY(EditAnywhere, Category="Sky")
	float NightExposure = -1.6f;

	/** Normal fog density, and how many times thicker it gets in full morning mist (from the seasons) */
	UPROPERTY(EditAnywhere, Category="Sky")
	float FogDensity = 0.03f;

	UPROPERTY(EditAnywhere, Category="Sky")
	float MistFogMultiplier = 6.f;

	/** Cloud material coverage for a clear sky and a fully overcast one */
	UPROPERTY(EditAnywhere, Category="Weather")
	FVector2D CloudCoverageRange = FVector2D(-0.8f, 0.7f);

	/** How much sunlight still gets through a fully overcast sky */
	UPROPERTY(EditAnywhere, Category="Weather", meta=(ClampMin=0, ClampMax=1))
	float OvercastSunlight = 0.12f;

	/** How much the camera opens up on grey days (EV at full overcast): eyes adapt, so overcast looks grey, not dark */
	UPROPERTY(EditAnywhere, Category="Weather")
	float OvercastExposureBoost = 2.2f;

	/** How much darker everything gets in heavy rain (share of the light) */
	UPROPERTY(EditAnywhere, Category="Weather", meta=(ClampMin=0, ClampMax=1))
	float RainDarkening = 0.35f;

	/** Makes the clouds follow the weather (a per-actor copy of the cloud material) */
	UPROPERTY(Transient)
	TObjectPtr<class UMaterialInstanceDynamic> CloudMaterial;

	/** Weather last applied (smoothed so the sky changes gradually) */
	float Overcast = 0.5f;
	float RainHaze = 0.f;

	/** The sky starts in the current weather instead of fading in from an average */
	bool bWeatherApplied = false;

	/** 0 at night .. 1 in full daylight, from the sun's height */
	float DayLight = 1.f;

	virtual void BeginPlay() override;

private:

	/** Positions the sun and moon and sets light levels for an astronomical date and solar hour */
	void UpdateSky(const FDateTime& AstronomicalDate, float SolarHour);
};

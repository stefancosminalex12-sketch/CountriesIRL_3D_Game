// CountriesIRL 3D Game

#include "World/DayNightSky.h"
#include "World/SkyMath.h"
#include "World/WorldClockSubsystem.h"
#include "World/WorldSimulationSettings.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ADayNightSky::ADayNightSky()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(RootComponent);
	Sun->SetMobility(EComponentMobility::Movable);
	Sun->SetAtmosphereSunLight(true);
	Sun->SetAtmosphereSunLightIndex(0);
	Sun->SetIntensity(SunIntensity);
	Sun->LightSourceAngle = 0.53f;

	Moon = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Moon"));
	Moon->SetupAttachment(RootComponent);
	Moon->SetMobility(EComponentMobility::Movable);
	Moon->SetAtmosphereSunLight(true);
	Moon->SetAtmosphereSunLightIndex(1);
	Moon->SetIntensity(0.f);
	Moon->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f));
	Moon->LightSourceAngle = 0.52f;

	Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmosphere"));
	Atmosphere->SetupAttachment(RootComponent);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->bRealTimeCapture = true;
	SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CloudMaterial(TEXT("/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud_Inst.m_SimpleVolumetricCloud_Inst"));
	Clouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("Clouds"));
	Clouds->SetupAttachment(RootComponent);
	Clouds->SetMaterial(CloudMaterial.Object);

	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
	Fog->SetupAttachment(RootComponent);
	Fog->SetFogDensity(0.03f);
	Fog->SetFogHeightFalloff(0.2f);

	// Exposure is set directly from the sun height (min = max) so day and night look deliberate
	// instead of the camera auto-adjusting night into day
	Exposure = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Exposure"));
	Exposure->SetupAttachment(RootComponent);
	Exposure->bUnbound = true;
	Exposure->Settings.bOverride_AutoExposureMinBrightness = true;
	Exposure->Settings.bOverride_AutoExposureMaxBrightness = true;
	Exposure->Settings.AutoExposureMinBrightness = DayExposure;
	Exposure->Settings.AutoExposureMaxBrightness = DayExposure;
}

void ADayNightSky::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Editor preview: the start date at PreviewHour
	const UWorldSimulationSettings* Settings = GetDefault<UWorldSimulationSettings>();
	UpdateSky(Settings->StartDateTime + FTimespan::FromDays(Settings->JulianToGregorianDays), PreviewHour);
}

void ADayNightSky::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (const UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>())
	{
		UpdateSky(Clock->GetAstronomicalDateTime(), Clock->GetTimeOfDay());
	}
}

void ADayNightSky::UpdateSky(const FDateTime& AstronomicalDate, float SolarHour)
{
	const double Latitude = GetDefault<UWorldSimulationSettings>()->Latitude;
	const int32 DayOfYear = AstronomicalDate.GetDayOfYear();

	const SkyMath::FSkyPosition SunPos = SkyMath::SunPosition(Latitude, DayOfYear, SolarHour);
	const double Phase = SkyMath::MoonPhase(AstronomicalDate);
	const SkyMath::FSkyPosition MoonPos = SkyMath::MoonPosition(Latitude, DayOfYear, SolarHour, Phase);

	// Lights shine from the body toward the ground
	Sun->SetWorldRotation((-SunPos.ToDirection()).Rotation());
	Moon->SetWorldRotation((-MoonPos.ToDirection()).Rotation());

	// Sun fades out just below the horizon so it never lights the world from underground
	const float SunUp = FMath::SmoothStep(-4.f, 2.f, static_cast<float>(SunPos.ElevationDeg));
	Sun->SetIntensity(SunIntensity * SunUp);
	Sun->SetCastShadows(SunUp > 0.f);

	// Moonlight only matters once the sun is well down; brightness follows the phase
	const float MoonUp = FMath::SmoothStep(-3.f, 3.f, static_cast<float>(MoonPos.ElevationDeg));
	const float Night = 1.f - FMath::SmoothStep(-10.f, 0.f, static_cast<float>(SunPos.ElevationDeg));
	const float Lit = FMath::Max(static_cast<float>(SkyMath::MoonIllumination(Phase)), MinMoonlight);
	const float MoonLight = MoonIntensity * Lit * MoonUp * Night;
	Moon->SetIntensity(MoonLight);
	Moon->SetCastShadows(MoonLight > 0.f);

	// Exposure eases from night to day over a long twilight, so low-sun dawns/dusks in shade stay readable
	const float DayAmount = FMath::SmoothStep(-8.f, 15.f, static_cast<float>(SunPos.ElevationDeg));
	const float EV = FMath::Lerp(NightExposure, DayExposure, DayAmount);
	Exposure->Settings.AutoExposureMinBrightness = EV;
	Exposure->Settings.AutoExposureMaxBrightness = EV;
}

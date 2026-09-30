// CountriesIRL 3D Game

#include "World/DayNightSky.h"
#include "World/SkyMath.h"
#include "World/WeatherSubsystem.h"
#include "World/PrecipitationComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
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
	// The sun is the main light for fog, water and translucency; the moon only takes over when it is alone
	Sun->ForwardShadingPriority = 1;

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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CloudMaterialFinder(TEXT("/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud_Inst.m_SimpleVolumetricCloud_Inst"));
	Clouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("Clouds"));
	Clouds->SetupAttachment(RootComponent);
	Clouds->SetMaterial(CloudMaterialFinder.Object);

	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
	Fog->SetupAttachment(RootComponent);
	Fog->SetFogDensity(FogDensity);
	Fog->SetFogHeightFalloff(0.2f);

	// Rain streaks and snowflakes (materials from Tools/Unreal/make_season_assets.py)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RainMaterial(TEXT("/Game/CrownsAndCommoners/World/M_Rain.M_Rain"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SnowMaterial(TEXT("/Game/CrownsAndCommoners/World/M_Snow.M_Snow"));

	Rain = CreateDefaultSubobject<UPrecipitationComponent>(TEXT("Rain"));
	Rain->SetupAttachment(RootComponent);
	Rain->SetStaticMesh(CubeMesh.Object);
	Rain->SetMaterial(0, RainMaterial.Object);
	Rain->MaxDrops = 7000;
	Rain->AreaRadius = 900.f;
	Rain->AreaHeight = 1200.f;

	Snow = CreateDefaultSubobject<UPrecipitationComponent>(TEXT("Snow"));
	Snow->SetupAttachment(RootComponent);
	Snow->SetStaticMesh(SphereMesh.Object);
	Snow->SetMaterial(0, SnowMaterial.Object);
	Snow->bSnow = true;
	Snow->MaxDrops = 6000;
	Snow->AreaRadius = 800.f;
	Snow->AreaHeight = 1000.f;
	Snow->FallSpeed = 110.f;
	Snow->WindInfluence = 0.8f;
	Snow->DropSize = FVector(3.5f);

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

void ADayNightSky::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInterface* Base = Clouds->Material.LoadSynchronous())
	{
		CloudMaterial = UMaterialInstanceDynamic::Create(Base, this);
		Clouds->SetMaterial(CloudMaterial);
	}
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

	// Weather: clouds build up and clear gradually, overcast skies dim the sun, rain and mist thicken the air
	if (const UWeatherSubsystem* Weather = GetWorld()->GetSubsystem<UWeatherSubsystem>())
	{
		const FWeatherState& Now = Weather->GetState();
		Overcast = bWeatherApplied ? FMath::FInterpTo(Overcast, Now.CloudCover, DeltaTime, 0.4f) : Now.CloudCover;
		RainHaze = bWeatherApplied ? FMath::FInterpTo(RainHaze, Now.Precipitation, DeltaTime, 0.5f) : Now.Precipitation;
		bWeatherApplied = true;

		if (CloudMaterial)
		{
			CloudMaterial->SetScalarParameterValue(TEXT("Cloud_GlobalCoverage"), FMath::Lerp(CloudCoverageRange.X, CloudCoverageRange.Y, Overcast));
			CloudMaterial->SetScalarParameterValue(TEXT("StormClouds"), FMath::Clamp(RainHaze * 1.2f, 0.f, 1.f));
		}

		// Thick cloud blocks nearly all direct sunlight (no sharp shadows); rain darkens everything a bit more
		const float Sunlight = FMath::Lerp(1.f, OvercastSunlight, FMath::Square(Overcast)) * (1.f - RainDarkening * RainHaze);
		Sun->SetIntensity(Sun->Intensity * Sunlight);
		// Light through cloud comes from a big bright patch of sky instead of a small sun: soft shadows
		Sun->SetLightSourceAngle(FMath::Lerp(0.53f, 12.f, Overcast));
		Moon->SetIntensity(Moon->Intensity * Sunlight);
		SkyLight->SetIntensity(1.f - RainDarkening * RainHaze);

		const float Mist = 1.f + (MistFogMultiplier - 1.f) * Now.Fog;
		Fog->SetFogDensity(FogDensity * Mist * (1.f + 2.f * RainHaze + 0.5f * Overcast));

		// How bright the day is for glowing things like rain and snow (0 night .. 1 bright day)
		if (UMaterialParameterCollection* Parameters = GetDefault<UWorldSimulationSettings>()->SeasonParameters.LoadSynchronous())
		{
			if (UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(Parameters))
			{
				Instance->SetScalarParameterValue(TEXT("Daylight"), DayLight * (1.f - 0.4f * Overcast));
			}
		}

		// Overcast days are darker, but the camera adapts a little (like our eyes do)
		const float EV = Exposure->Settings.AutoExposureMinBrightness - OvercastExposureBoost * Overcast - 0.6f * RainHaze;
		Exposure->Settings.AutoExposureMinBrightness = EV;
		Exposure->Settings.AutoExposureMaxBrightness = EV;
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
	DayLight = DayAmount;
	const float EV = FMath::Lerp(NightExposure, DayExposure, DayAmount);
	Exposure->Settings.AutoExposureMinBrightness = EV;
	Exposure->Settings.AutoExposureMaxBrightness = EV;
}

// CountriesIRL 3D Game

#include "UI/CIRLPaperDollStage.h"
#include "Characters/BallCharacter.h"
#include "Characters/BallAnimatorComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/CapsuleComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	/** Where the camera stands, relative to the ground under the doll (cm): in front of it, at waist height */
	const FVector CameraOffset(430.f, 0.f, 88.f);

	UPointLightComponent* MakeStudioLight(AActor* Owner, const TCHAR* Name, const FVector& Location, float Candelas, const FLinearColor& Color, bool bShadows)
	{
		UPointLightComponent* Light = Owner->CreateDefaultSubobject<UPointLightComponent>(Name);
		Light->SetupAttachment(Owner->GetRootComponent());
		Light->SetRelativeLocation(Location);
		Light->SetIntensityUnits(ELightUnits::Candelas);
		Light->SetIntensity(Candelas);
		Light->SetLightColor(Color);
		Light->SetAttenuationRadius(1200.f);
		Light->SetCastShadows(bShadows);
		// Studio lights only light the doll (channel 1), nothing in the world
		Light->LightingChannels.bChannel0 = false;
		Light->LightingChannels.bChannel1 = true;
		return Light;
	}
}

ACIRLPaperDollStage::ACIRLPaperDollStage()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	// Doll first (it animates), then the picture
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
	Capture->SetupAttachment(GetRootComponent());
	Capture->SetRelativeLocationAndRotation(CameraOffset, FRotator(0.f, 180.f, 0.f));
	Capture->FOVAngle = 27.f;
	Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	Capture->bCaptureEveryFrame = false;
	Capture->bCaptureOnMovement = false;
	// Just the doll: no fog, sky or motion blur from the world
	Capture->ShowFlags.SetFog(false);
	Capture->ShowFlags.SetVolumetricFog(false);
	Capture->ShowFlags.SetAtmosphere(false);
	Capture->ShowFlags.SetCloud(false);
	Capture->ShowFlags.SetMotionBlur(false);
	Capture->ShowFlags.SetBloom(false);
	// Fixed exposure, so the doll is equally bright at noon and at midnight
	Capture->PostProcessSettings.bOverride_AutoExposureMethod = true;
	Capture->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	Capture->PostProcessSettings.bOverride_AutoExposureBias = true;
	Capture->PostProcessSettings.AutoExposureBias = 8.5f;

	KeyLight = MakeStudioLight(this, TEXT("KeyLight"), FVector(260.f, -220.f, 260.f), 900.f, FLinearColor(1.f, 0.9f, 0.78f), true);
	FillLight = MakeStudioLight(this, TEXT("FillLight"), FVector(240.f, 260.f, 110.f), 300.f, FLinearColor(0.78f, 0.85f, 1.f), false);
	RimLight = MakeStudioLight(this, TEXT("RimLight"), FVector(-240.f, 60.f, 280.f), 600.f, FLinearColor(1.f, 0.92f, 0.8f), false);

	// Keeps filming and animating while the game is paused behind the menu
	SetTickableWhenPaused(true);
}

void ACIRLPaperDollStage::BeginPlay()
{
	Super::BeginPlay();

	RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("PaperDollTarget"));
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8_SRGB;
	RenderTarget->ClearColor = FLinearColor::Transparent;
	RenderTarget->InitAutoFormat(Resolution, Resolution);
	RenderTarget->UpdateResourceImmediate(true);
	Capture->TextureTarget = RenderTarget;

	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/CountriesIRL/UI/Materials/M_UI_PaperDoll.M_UI_PaperDoll")))
	{
		Picture = UMaterialInstanceDynamic::Create(Base, this);
		Picture->SetTextureParameterValue(TEXT("Picture"), RenderTarget);
	}

	// The doll: a plain ball that never moves or collides; for now it looks like the default player ball
	// (flag, livery and worn equipment get copied onto it once those exist)
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Doll = GetWorld()->SpawnActor<ABallCharacter>(ABallCharacter::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, Params);
	if (!Doll)
	{
		return;
	}
	Doll->Tags.Add(TEXT("PaperDoll"));
	Doll->SetActorEnableCollision(false);
	UCharacterMovementComponent* Movement = Doll->GetCharacterMovement();
	Movement->GravityScale = 0.f;
	Movement->DisableMovement();
	Doll->SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, Doll->GetGroundOffset()));

	Doll->SetTickableWhenPaused(true);
	for (UActorComponent* Component : Doll->GetComponents())
	{
		Component->SetTickableWhenPaused(true);
		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component))
		{
			Primitive->SetLightingChannels(false, true, false);
		}
	}

	Capture->ShowOnlyActors.Add(Doll);
	ApplyDollYaw();
}

void ACIRLPaperDollStage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Doll)
	{
		Doll->Destroy();
		Doll = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ACIRLPaperDollStage::AddDollYaw(float Degrees)
{
	DollYaw = FRotator::NormalizeAxis(DollYaw + Degrees);
	ApplyDollYaw();
}

void ACIRLPaperDollStage::ResetDollYaw()
{
	DollYaw = 0.f;
	ApplyDollYaw();
}

void ACIRLPaperDollStage::SetCapturing(bool bCapture)
{
	bCapturing = bCapture;
	SetActorTickEnabled(bCapture);
	if (bCapture)
	{
		Capture->CaptureScene();
	}
}

void ACIRLPaperDollStage::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bCapturing)
	{
		Capture->CaptureScene();
	}
}

void ACIRLPaperDollStage::ApplyDollYaw()
{
	if (Doll)
	{
		// Yaw 0 looks straight at the camera (which stands on the +X side)
		Doll->SetActorRotation(FRotator(0.f, RestingYaw + DollYaw, 0.f));
		// Turned like a statue on a turntable: the feet turn with the body instead of stepping around
		Doll->GetAnimator()->SnapFeetToRest();
	}
}

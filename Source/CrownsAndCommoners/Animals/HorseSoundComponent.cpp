// Crowns & Commoners

#include "Animals/HorseSoundComponent.h"
#include "Animals/Horse.h"
#include "Animals/MountDefinition.h"
#include "Characters/StaminaComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"

UHorseSoundComponent::UHorseSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHorseSoundComponent::BeginPlay()
{
	Super::BeginPlay();

	AHorse* Horse = Cast<AHorse>(GetOwner());
	const UMountDefinition* Definition = Horse ? Horse->GetDefinition() : nullptr;
	if (!Definition)
	{
		return;
	}

	// Loud close by, fading out over the distance
	Attenuation = NewObject<USoundAttenuation>(this);
	Attenuation->Attenuation.bAttenuate = true;
	Attenuation->Attenuation.bSpatialize = true;
	Attenuation->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	Attenuation->Attenuation.AttenuationShapeExtents = FVector(300.f, 0.f, 0.f);
	Attenuation->Attenuation.FalloffDistance = HearingDistance;

	USoundBase* Loops[] = { Definition->WalkSound, Definition->TrotSound, Definition->CanterSound, Definition->GallopSound };
	for (USoundBase* Sound : Loops)
	{
		UAudioComponent* Loop = NewObject<UAudioComponent>(Horse);
		Loop->SetupAttachment(Horse->GetMesh());
		Loop->bAutoActivate = false;
		Loop->bAutoDestroy = false;
		Loop->SetSound(Sound);
		Loop->AttenuationSettings = Attenuation;
		Loop->RegisterComponent();
		GaitLoops.Add(Loop);
	}
	NextSnort = FMath::FRandRange(RestedSnortGap.X * 0.3f, RestedSnortGap.Y);
}

void UHorseSoundComponent::PlayOneShot(const TArray<TObjectPtr<USoundBase>>& Sounds)
{
	AHorse* Horse = Cast<AHorse>(GetOwner());
	if (!Horse || Sounds.Num() == 0)
	{
		return;
	}
	if (USoundBase* Sound = Sounds[FMath::RandHelper(Sounds.Num())])
	{
		// From the head end of the horse, a little different every time
		UGameplayStatics::SpawnSoundAttached(Sound, Horse->GetMesh(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset,
			false, 1.f, FMath::FRandRange(0.94f, 1.06f), 0.f, Attenuation);
	}
}

float UHorseSoundComponent::PlayNeigh()
{
	const AHorse* Horse = Cast<AHorse>(GetOwner());
	const UMountDefinition* Definition = Horse ? Horse->GetDefinition() : nullptr;
	if (!Definition || Definition->NeighSounds.Num() == 0)
	{
		return 0.f;
	}
	USoundBase* Sound = Definition->NeighSounds[FMath::RandHelper(Definition->NeighSounds.Num())];
	if (!Sound)
	{
		return 0.f;
	}
	UGameplayStatics::SpawnSoundAttached(Sound, Horse->GetMesh(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset,
		false, 1.f, FMath::FRandRange(0.96f, 1.04f), 0.f, Attenuation);
	return Sound->GetDuration();
}

void UHorseSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AHorse* Horse = Cast<AHorse>(GetOwner());
	const UMountDefinition* Definition = Horse ? Horse->GetDefinition() : nullptr;
	if (!Definition || GaitLoops.Num() != 4)
	{
		return;
	}

	// Hoofbeats: the loop of the gait it is in, louder and a touch quicker the closer it is to that gait's full speed
	const float Speed = Horse->GetVelocity().Size2D();
	const bool bMoving = !Horse->IsDead() && Speed > 30.f && !Horse->GetCharacterMovement()->IsFalling();
	const int32 Current = static_cast<int32>(Horse->GetGait());
	const float GaitSpeed = FMath::Max(Definition->GetGaitSpeed(Horse->GetGait()), 1.f);
	const float Pace = FMath::Clamp(Speed / GaitSpeed, 0.f, 1.2f);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const float Target = (bMoving && Index == Current) ? FMath::Lerp(0.45f, 1.f, FMath::Min(Pace, 1.f)) : 0.f;
		GaitVolume[Index] = FMath::FInterpTo(GaitVolume[Index], Target, DeltaTime, 6.f);
		UAudioComponent* Loop = GaitLoops[Index];
		if (!Loop || !Loop->GetSound())
		{
			continue;
		}
		if (GaitVolume[Index] > 0.01f)
		{
			if (!Loop->IsPlaying())
			{
				Loop->Play(FMath::FRandRange(0.f, 1.f));
			}
			Loop->SetVolumeMultiplier(GaitVolume[Index]);
			Loop->SetPitchMultiplier(FMath::Lerp(0.9f, 1.05f, FMath::Min(Pace, 1.f)));
		}
		else if (Loop->IsPlaying())
		{
			Loop->Stop();
		}
	}

	// Snorts: now and then, and often when out of breath
	if (Horse->IsDead())
	{
		return;
	}
	NextSnort -= DeltaTime;
	if (NextSnort <= 0.f)
	{
		const bool bTired = Horse->GetStamina()->GetStaminaPercent() < TiredBelow || !Horse->GetStamina()->HasStamina();
		PlayOneShot(Definition->SnortSounds);
		const FVector2D Gap = bTired ? TiredSnortGap : RestedSnortGap;
		NextSnort = FMath::FRandRange(Gap.X, Gap.Y);
	}
}

// Crowns & Commoners

#include "Characters/BallFootstepsComponent.h"
#include "Characters/BallCharacter.h"
#include "Audio/CIRLMusicSettings.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/PhysicsSettings.h"

UBallFootstepsComponent::UBallFootstepsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBallFootstepsComponent::BeginPlay()
{
	Super::BeginPlay();
	ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball)
	{
		return;
	}
	for (TObjectPtr<UAudioComponent>& Player : Players)
	{
		Player = NewObject<UAudioComponent>(Ball);
		Player->SetupAttachment(Ball->GetCapsuleComponent());
		Player->bAutoActivate = false;
		Player->bAutoDestroy = false;
		Player->bAllowSpatialization = false;
		Player->RegisterComponent();
	}
}

FName UBallFootstepsComponent::TraceGround() const
{
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BallFootsteps), false, Ball);
	Params.bReturnPhysicalMaterial = true;
	const FVector Start = Ball->GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, Ball->GetGroundOffset() + 40.f);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return NAME_None;
	}
	const EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
	for (const FPhysicalSurfaceName& Named : UPhysicsSettings::Get()->PhysicalSurfaces)
	{
		if (Named.Type == Surface)
		{
			return Named.Name;
		}
	}
	return NAME_None;
}

void UBallFootstepsComponent::StartSteps(int32 Set)
{
	const TArray<FCIRLFootstepSet>& Sets = GetDefault<UCIRLMusicSettings>()->Footsteps;
	if (!Sets.IsValidIndex(Set) || Sets[Set].Sounds.Num() == 0)
	{
		return;
	}
	// A different variation from the last one, where there is a choice
	const int32 Count = Sets[Set].Sounds.Num();
	int32 Variation = FMath::RandHelper(Count);
	if (Count > 1 && Set == CurrentSet && Variation == LastVariation)
	{
		Variation = (Variation + 1 + FMath::RandHelper(Count - 1)) % Count;
	}
	USoundBase* Sound = Sets[Set].Sounds[Variation].LoadSynchronous();
	if (!Sound)
	{
		return;
	}

	if (UAudioComponent* Old = Players[ActivePlayer]; Old && Old->IsPlaying())
	{
		Old->FadeOut(0.2f, 0.f);
	}
	ActivePlayer = 1 - ActivePlayer;
	UAudioComponent* Player = Players[ActivePlayer];
	Player->SetSound(Sound);
	// Long recordings start anywhere, so walking never sounds the same twice
	const float Start = FMath::FRandRange(0.f, FMath::Max(Sound->GetDuration() - 3.f, 0.f));
	Player->FadeIn(0.15f, 1.f, Start);
	CurrentSet = Set;
	LastVariation = Variation;
}

void UBallFootstepsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball || !Players[0])
	{
		return;
	}

	const float Speed = Ball->GetVelocity().Size2D();
	const bool bWalking = !Ball->IsDead() && !Ball->IsMounted() && Speed > 20.f && Ball->GetCharacterMovement()->IsMovingOnGround();
	const float Pace = Speed / 220.f;
	const float Target = bWalking ? FMath::Clamp(Pace, 0.35f, 1.3f) * (Ball->IsSneaking() ? 0.35f : 1.f) : 0.f;
	Volume = FMath::FInterpTo(Volume, Target, DeltaTime, 10.f);

	if (!bWalking)
	{
		if (bStepping && Volume < 0.02f)
		{
			Players[ActivePlayer]->FadeOut(0.15f, 0.f);
			bStepping = false;
		}
	}
	else
	{
		// Which ground: checked a few times a second
		NextGroundCheck -= DeltaTime;
		if (NextGroundCheck <= 0.f)
		{
			NextGroundCheck = 0.25f;
			Ground = TraceGround();
		}
		const TArray<FCIRLFootstepSet>& Sets = GetDefault<UCIRLMusicSettings>()->Footsteps;
		const int32 Found = Sets.IndexOfByPredicate([this](const FCIRLFootstepSet& Set) { return Set.Surface == Ground; });
		const int32 Set = Found != INDEX_NONE ? Found : 0;
		// A new variation each time we set off, and a new ground's own sound as soon as we step onto it
		if (!bStepping || Set != CurrentSet)
		{
			StartSteps(Set);
			bStepping = true;
		}
	}

	if (UAudioComponent* Player = Players[ActivePlayer]; Player && bStepping)
	{
		Player->SetVolumeMultiplier(FMath::Max(Volume, 0.01f));
		Player->SetPitchMultiplier(FMath::Clamp(FMath::Sqrt(FMath::Max(Pace, 0.3f)), 0.75f, 1.35f));
	}
}

void UBallFootstepsComponent::OnLanded(float FallSpeed)
{
	if (FallSpeed < LandingSpeed.X)
	{
		return;
	}
	const TArray<TSoftObjectPtr<USoundBase>>& Sounds = GetDefault<UCIRLMusicSettings>()->LandingSounds;
	if (Sounds.Num() == 0)
	{
		return;
	}
	if (USoundBase* Sound = Sounds[FMath::RandHelper(Sounds.Num())].LoadSynchronous())
	{
		const float Strength = FMath::GetMappedRangeValueClamped(LandingSpeed, FVector2D(0.4f, 1.f), FallSpeed);
		UGameplayStatics::PlaySound2D(this, Sound, Strength, FMath::FRandRange(0.95f, 1.05f));
	}
}

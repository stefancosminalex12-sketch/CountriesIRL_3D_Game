// Crowns & Commoners

#include "Characters/BallHeldItemsComponent.h"
#include "Characters/BallAnimatorComponent.h"
#include "Characters/BallCharacter.h"
#include "Characters/BallParts.h"
#include "Items/CIRLInventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

namespace
{
	const FLinearColor Wood(0.30f, 0.19f, 0.10f);
	const FLinearColor Steel(0.55f, 0.57f, 0.60f);
	const FLinearColor Leather(0.22f, 0.13f, 0.07f);
	const FLinearColor PaintedBoard(0.45f, 0.12f, 0.10f);
	const FLinearColor Horn(0.85f, 0.72f, 0.40f);
	const FLinearColor Flame(1.0f, 0.45f, 0.08f);
	const FLinearColor Yew(0.52f, 0.36f, 0.16f);

	/** Where the fist grips, in the hand's own space (X along the fingers, Z out of the back of the hand) */
	const FVector GripInHand(7.f, 0.f, -4.f);

	/** A weapon leaves the fist on the thumb side, tilted this far back toward the wrist (so a hanging hand
	 *  carries it pointing forward and up, not at the ground) */
	constexpr float CarryTiltDegrees = 35.f;
}

void UBallHeldItemsComponent::BeginPlay()
{
	Super::BeginPlay();

	CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball)
	{
		return;
	}
	for (int32 Index = 0; Index < 2; ++Index)
	{
		USceneComponent* HandRoot = Ball->GetAnimator()->GetHandRoot(Index);
		if (!HandRoot)
		{
			continue;
		}
		USceneComponent* Holder = NewObject<USceneComponent>(Ball);
		Holder->SetupAttachment(HandRoot);
		// True centimetres, whatever size the cartoon hands are drawn at
		Holder->SetUsingAbsoluteScale(true);
		Holder->RegisterComponent();
		Holders.Add(Holder);
		Hands[Index].Holder = Holder;
	}

	Ball->GetInventory()->OnChanged.AddUObject(this, &UBallHeldItemsComponent::Refresh);
	Refresh();
}

void UBallHeldItemsComponent::Refresh()
{
	ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball)
	{
		return;
	}
	const UCIRLInventoryComponent* Things = Ball->GetInventory();

	// Hand 0 is the left (off hand), hand 1 the right (main hand)
	const FCIRLItemRow* Items[2] = {
		Things->IsSlotBlocked(ECIRLEquipSlot::WeaponOff) ? nullptr : Things->FindItem(Things->GetEquipped(ECIRLEquipSlot::WeaponOff)),
		Things->FindItem(MainHandOverride.IsNone() ? Things->GetEquipped(ECIRLEquipSlot::WeaponMain) : MainHandOverride)
	};
	for (int32 Index = 0; Index < 2; ++Index)
	{
		FHeld& Hand = Hands[Index];
		if (!Hand.Holder)
		{
			continue;
		}
		Hand.Used = 0;
		// The thumb is on the side of the hand facing the body
		Build(Hand, Items[Index], Index == 0 ? 1.f : -1.f);
		for (int32 Part = Hand.Used; Part < Hand.Parts.Num(); ++Part)
		{
			Hand.Parts[Part]->SetVisibility(false);
		}
		Ball->GetAnimator()->SetHolding(Index, Items[Index] != nullptr && Items[Index]->Shape != ECIRLItemShape::None);
	}
}

void UBallHeldItemsComponent::SetMainHandOverride(FName ItemId)
{
	if (MainHandOverride != ItemId)
	{
		MainHandOverride = ItemId;
		Refresh();
	}
}

void UBallHeldItemsComponent::AddPart(FHeld& Hand, EPartMesh Mesh, const FVector& Center, const FVector& Size, const FLinearColor& Color, const FRotator& Rotation)
{
	if (Hand.Used >= Hand.Parts.Num())
	{
		UStaticMeshComponent* NewPart = NewObject<UStaticMeshComponent>(GetOwner());
		NewPart->SetupAttachment(Hand.Holder);
		NewPart->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		NewPart->SetGenerateOverlapEvents(false);
		NewPart->SetCanEverAffectNavigation(false);
		// Lit the way the rest of the ball is (the Equipment screen's character has its own studio lights)
		if (const UStaticMeshComponent* Body = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
		{
			NewPart->SetLightingChannels(Body->LightingChannels.bChannel0, Body->LightingChannels.bChannel1, Body->LightingChannels.bChannel2);
		}
		NewPart->RegisterComponent();
		AllParts.Add(NewPart);
		Hand.Parts.Add(NewPart);
	}
	UStaticMeshComponent* Part = Hand.Parts[Hand.Used++];
	Part->SetStaticMesh(Mesh == EPartMesh::Cylinder ? CylinderMesh : (Mesh == EPartMesh::Cube ? CubeMesh : SphereMesh));
	Part->SetRelativeLocationAndRotation(Center, Rotation);
	// The engine's shapes are 100 cm across
	Part->SetRelativeScale3D(Size / 100.f);
	Part->SetVisibility(true);
	BallParts::SetColor(Part, Color);
}

void UBallHeldItemsComponent::Build(FHeld& Hand, const FCIRLItemRow* Item, float ThumbSide)
{
	if (!Item || Item->Shape == ECIRLItemShape::None)
	{
		return;
	}

	// The holder's Z runs along the weapon: out of the thumb side of the fist, tilted back toward the wrist.
	// Its X points out of the back of the hand (where a shield sits)
	const float Tilt = FMath::DegreesToRadians(CarryTiltDegrees);
	const FVector Along(-FMath::Sin(Tilt), ThumbSide * FMath::Cos(Tilt), 0.f);
	Hand.Holder->SetRelativeLocationAndRotation(GripInHand, FRotationMatrix::MakeFromZX(Along, FVector::UpVector).Rotator());

	using M = EPartMesh;
	const float L = FMath::Max(Item->ReachCm, 10.f);
	switch (Item->Shape)
	{
	case ECIRLItemShape::Pole:
	{
		// Held a third of the way up the shaft; the head depends on what the weapon does
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, L * 0.2f), FVector(3.5f, 3.5f, L), Wood);
		if (Item->DamageType == ECIRLDamageType::Pierce)
		{
			AddPart(Hand, M::Cube, FVector(0.f, 0.f, L * 0.7f + 10.f), FVector(1.5f, 5.f, 24.f), Steel);
		}
		else if (Item->DamageType == ECIRLDamageType::Blunt)
		{
			AddPart(Hand, M::Cube, FVector(0.f, 0.f, L * 0.7f - 8.f), FVector(6.f, 16.f, 8.f), Steel);
			AddPart(Hand, M::Cube, FVector(0.f, 0.f, L * 0.7f + 6.f), FVector(1.5f, 2.5f, 20.f), Steel);
		}
		else
		{
			AddPart(Hand, M::Cube, FVector(0.f, 3.f, L * 0.7f - 6.f), FVector(1.5f, 10.f, 32.f), Steel);
			AddPart(Hand, M::Cube, FVector(0.f, 0.f, L * 0.7f + 16.f), FVector(1.5f, 2.5f, 16.f), Steel);
		}
		break;
	}
	case ECIRLItemShape::Staff:
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, L * 0.1f), FVector(3.5f, 3.5f, L), Wood);
		break;
	case ECIRLItemShape::Blade:
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, 0.f), FVector(3.f, 3.f, 16.f), Leather);
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, 9.f), FVector(2.5f, 18.f, 2.5f), Steel);
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, 10.f + (L - 18.f) * 0.5f), FVector(1.2f, 4.5f, L - 18.f), Steel);
		break;
	case ECIRLItemShape::Dagger:
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, 0.f), FVector(2.6f, 2.6f, 11.f), Leather);
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, 6.5f), FVector(2.f, 6.f, 1.5f), Steel);
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, 7.f + (L - 12.f) * 0.5f), FVector(1.f, 2.6f, L - 12.f), Steel);
		break;
	case ECIRLItemShape::Club:
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, L * 0.5f - 10.f), FVector(5.f, 5.f, L), Wood);
		break;
	case ECIRLItemShape::Axe:
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, L * 0.5f - 12.f), FVector(3.5f, 3.5f, L), Wood);
		AddPart(Hand, M::Cube, FVector(0.f, 7.f, L - 20.f), FVector(2.5f, 15.f, 12.f), Steel);
		break;
	case ECIRLItemShape::Hammer:
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, L * 0.5f - 10.f), FVector(3.f, 3.f, L), Wood);
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, L - 14.f), FVector(6.f, 12.f, 6.f), Steel);
		break;
	case ECIRLItemShape::Bow:
		// Held at the middle; the string runs behind the stave
		AddPart(Hand, M::Cylinder, FVector::ZeroVector, FVector(2.8f, 2.8f, L), Yew);
		AddPart(Hand, M::Cylinder, FVector(-7.f, 0.f, 0.f), FVector(0.5f, 0.5f, L - 6.f), FLinearColor(0.8f, 0.75f, 0.6f));
		break;
	case ECIRLItemShape::Crossbow:
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, L * 0.3f), FVector(4.f, 5.f, L), Wood);
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, L * 0.7f), FVector(2.5f, 70.f, 3.f), Steel);
		break;
	case ECIRLItemShape::Shield:
		// A board on the back of the hand
		AddPart(Hand, M::Cube, FVector(9.f, 0.f, 4.f), FVector(4.f, L * 0.7f, L), PaintedBoard);
		break;
	case ECIRLItemShape::Buckler:
		// A small round shield in the fist (the disc faces out of the back of the hand)
		AddPart(Hand, M::Cylinder, FVector(8.f, 0.f, 0.f), FVector(L, L, 3.f), Steel, FRotator(90.f, 0.f, 0.f));
		AddPart(Hand, M::Sphere, FVector(10.f, 0.f, 0.f), FVector(10.f, 10.f, 10.f), Steel);
		break;
	case ECIRLItemShape::Torch:
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, L * 0.5f - 10.f), FVector(3.5f, 3.5f, L), Wood);
		AddPart(Hand, M::Sphere, FVector(0.f, 0.f, L - 6.f), FVector(11.f, 11.f, 15.f), Flame);
		break;
	case ECIRLItemShape::Lantern:
		// Hangs below the fist from its ring
		AddPart(Hand, M::Cylinder, FVector(0.f, 0.f, -8.f), FVector(1.f, 1.f, 12.f), Steel);
		AddPart(Hand, M::Cube, FVector(0.f, 0.f, -24.f), FVector(12.f, 12.f, L * 0.8f), Horn);
		break;
	default:
		break;
	}
}

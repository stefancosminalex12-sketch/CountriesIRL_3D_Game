// Crowns & Commoners

#include "Characters/BallWornGearComponent.h"
#include "Characters/BallCharacter.h"
#include "Characters/BallParts.h"
#include "Items/CIRLInventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

namespace
{
	const FLinearColor Leather(0.22f, 0.13f, 0.07f);
	const FLinearColor Steel(0.55f, 0.57f, 0.60f);
	const FLinearColor Wood(0.30f, 0.19f, 0.10f);
}

void UBallWornGearComponent::BeginPlay()
{
	Super::BeginPlay();

	SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner()))
	{
		R = Ball->GetBallRadius();
		Ball->GetInventory()->OnChanged.AddUObject(this, &UBallWornGearComponent::Refresh);
		Refresh();
	}
}

void UBallWornGearComponent::SetOwnerNoSee(bool bHide)
{
	bOwnerNoSee = bHide;
	for (UStaticMeshComponent* Part : Parts)
	{
		Part->SetOwnerNoSee(bHide);
	}
}

void UBallWornGearComponent::SetShown(bool bShow)
{
	if (bShown != bShow)
	{
		bShown = bShow;
		Refresh();
	}
}

void UBallWornGearComponent::AddPart(EPartMesh Mesh, const FVector& Center, const FVector& Size, const FLinearColor& Color, const FRotator& Rotation)
{
	ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball || !Ball->GetBodyPivot())
	{
		return;
	}
	if (Used >= Parts.Num())
	{
		UStaticMeshComponent* NewPart = NewObject<UStaticMeshComponent>(Ball);
		// On the body itself: it leans, bobs and falls over with the ball
		NewPart->SetupAttachment(Ball->GetBodyPivot());
		NewPart->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		NewPart->SetGenerateOverlapEvents(false);
		NewPart->SetCanEverAffectNavigation(false);
		NewPart->SetOwnerNoSee(bOwnerNoSee);
		NewPart->bCastHiddenShadow = true;
		// Lit the way the rest of the ball is (the Equipment screen's character has its own studio lights)
		if (const UStaticMeshComponent* Body = Ball->FindComponentByClass<UStaticMeshComponent>())
		{
			NewPart->SetLightingChannels(Body->LightingChannels.bChannel0, Body->LightingChannels.bChannel1, Body->LightingChannels.bChannel2);
		}
		NewPart->RegisterComponent();
		Parts.Add(NewPart);
	}
	UStaticMeshComponent* Part = Parts[Used++];
	Part->SetStaticMesh(Mesh == EPartMesh::Sphere ? SphereMesh : (Mesh == EPartMesh::Cylinder ? CylinderMesh : CubeMesh));
	Part->SetRelativeLocationAndRotation(Center, Rotation);
	// The engine's shapes are 100 cm across
	Part->SetRelativeScale3D(Size / 100.f);
	Part->SetVisibility(true);
	BallParts::SetColor(Part, Color);
}

void UBallWornGearComponent::AddShell(float CenterX, float CenterZ, float HalfLength, float HalfWidth, float HalfHeight, const FLinearColor& Color)
{
	AddPart(EPartMesh::Sphere, FVector(CenterX, 0.f, CenterZ) * R, FVector(HalfLength, HalfWidth, HalfHeight) * (2.f * R), Color);
}

void UBallWornGearComponent::Refresh()
{
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball)
	{
		return;
	}
	const UCIRLInventoryComponent* Things = Ball->GetInventory();
	Used = 0;

	auto Worn = [Things](ECIRLEquipSlot Slot) { return Things->FindItem(Things->GetEquipped(Slot)); };
	using S = ECIRLEquipSlot;

	if (bShown)
	{
		// Head: the coif or hood first, the helmet or hat over it
		const bool bHelmetOn = Worn(S::Helmet) != nullptr;
		for (const S Slot : { S::Coif, S::Helmet })
		{
			if (const FCIRLItemRow* Item = Worn(Slot))
			{
				BuildHead(*Item, Item->GetTint(Steel), Slot == S::Coif && bHelmetOn);
			}
		}

		// Body: from the skin outward. Each layer starts a little lower than the one under it, so they show as stripes
		for (const S Slot : { S::Tunic, S::Gambeson, S::Mail, S::Plate })
		{
			if (const FCIRLItemRow* Item = Worn(Slot))
			{
				BuildBody(Slot, *Item, Item->GetTint(Steel));
			}
		}

		// Cloak: draped over the back and shoulders, open at the front
		if (const FCIRLItemRow* Cloak = Worn(S::Cloak))
		{
			const FLinearColor Color = Cloak->GetTint(Leather);
			AddShell(-0.20f, -0.10f, 1.0f, 1.03f, 1.0f, Color);
			if (Cloak->Shape == ECIRLItemShape::CapeHood)
			{
				// The hood hangs down the back of the neck
				AddShell(-0.72f, 0.52f, 0.30f, 0.34f, 0.26f, Color);
			}
		}

		// Collar: a gold ring around the ball under the face
		if (const FCIRLItemRow* Collar = Worn(S::Necklace))
		{
			AddPart(EPartMesh::Cylinder, FVector(0.f, 0.f, -0.03f * R), FVector(2.10f * R, 2.10f * R, 3.f), Collar->GetTint(Steel));
		}

		// Belt: shown once something hangs from it; one half on the left hip, the other on the right
		const FCIRLItemRow* Belt1 = Worn(S::Belt1);
		const FCIRLItemRow* Belt2 = Worn(S::Belt2);
		if (Belt1 || Belt2)
		{
			AddPart(EPartMesh::Cylinder, FVector(0.f, 0.f, -0.32f * R), FVector(2.20f * R, 2.20f * R, 0.075f * R), Leather);
			AddPart(EPartMesh::Cube, FVector(1.10f * R, 0.f, -0.32f * R), FVector(3.f, 0.10f * R, 0.11f * R), Steel);
			if (Belt1)
			{
				BuildBeltItem(*Belt1, Belt1->GetTint(Leather), -1.f);
			}
			if (Belt2)
			{
				BuildBeltItem(*Belt2, Belt2->GetTint(Leather), 1.f);
			}
		}

		// Back: a shield or pavise slung behind, outside the cloak
		if (const FCIRLItemRow* Back = Worn(S::Back))
		{
			const float Height = FMath::Max(Back->ReachCm, 40.f);
			AddPart(EPartMesh::Cube, FVector(-1.24f * R, 0.f, 0.05f * R), FVector(5.f, Height * 0.68f, Height), Back->GetTint(Wood));
			AddPart(EPartMesh::Cube, FVector(-1.24f * R - 3.f, 0.f, 0.05f * R), FVector(2.f, Height * 0.68f + 4.f, 5.f), Steel);
		}
	}

	for (int32 Index = Used; Index < Parts.Num(); ++Index)
	{
		Parts[Index]->SetVisibility(false);
	}

	// Gloves and boots are the hands and boots themselves, in the item's colour
	const FCIRLItemRow* Gloves = Worn(S::Gloves);
	const FCIRLItemRow* Boots = Worn(S::Boots);
	CastChecked<ABallCharacter>(GetOwner())->SetGearTints(
		Gloves && !Gloves->Tint.IsEmpty() ? TOptional<FLinearColor>(Gloves->GetTint(Leather)) : TOptional<FLinearColor>(),
		Boots && !Boots->Tint.IsEmpty() ? TOptional<FLinearColor>(Boots->GetTint(Leather)) : TOptional<FLinearColor>());
}

void UBallWornGearComponent::AddCap(float RimHeight, float TopRoom, const FLinearColor& Color, float LengthScale, float ShiftBack)
{
	// A dome that sits on the ball from RimHeight (share of the radius above the centre) upward. At the rim it is a
	// little wider than the ball is there, and it rises TopRoom above the ball's top, so the ball never shows through
	const float BallAtRim = FMath::Sqrt(FMath::Max(1.f - RimHeight * RimHeight, 0.01f));
	const float HalfWidth = BallAtRim * 1.035f;
	const float HalfHeight = (1.f - RimHeight) + TopRoom;
	AddShell(-ShiftBack, RimHeight, HalfWidth * LengthScale, HalfWidth, HalfHeight, Color);
}

void UBallWornGearComponent::BuildHead(const FCIRLItemRow& Item, const FLinearColor& Color, bool bUnderHelmet)
{
	// A coif or hood under a helmet is covered by it: nothing of it is drawn (any part that stuck out clipped)
	if (bUnderHelmet)
	{
		return;
	}

	// Rim heights are shares of the ball's radius above its centre; the eyes are around 0.15
	switch (Item.Shape)
	{
	case ECIRLItemShape::KettleHat:
		AddCap(0.55f, 0.08f, Color);
		// The wide iron brim
		AddPart(EPartMesh::Cylinder, FVector(0.f, 0.f, 0.56f * R), FVector(2.30f * R, 2.30f * R, 3.f), Color);
		break;
	case ECIRLItemShape::Sallet:
		// Longer front to back, with the tail over the neck
		AddCap(0.48f, 0.08f, Color, 1.08f, 0.05f);
		break;
	case ECIRLItemShape::Armet:
		AddCap(0.40f, 0.10f, Color);
		// A low crest along the top
		AddPart(EPartMesh::Cube, FVector(-0.05f * R, 0.f, 1.10f * R), FVector(0.9f * R, 3.f, 0.10f * R), Color);
		break;
	case ECIRLItemShape::WideHat:
		// Perched high on top: a small crown and a very wide brim
		AddCap(0.72f, 0.07f, Color);
		AddPart(EPartMesh::Cylinder, FVector(0.f, 0.f, 0.73f * R), FVector(2.50f * R, 2.50f * R, 2.5f), Color);
		break;
	case ECIRLItemShape::Hat:
		AddCap(0.66f, 0.10f, Color);
		AddPart(EPartMesh::Cylinder, FVector(0.f, 0.f, 0.67f * R), FVector(1.95f * R, 1.95f * R, 3.f), Color);
		break;
	case ECIRLItemShape::Coif:
		// A close cap, lower than any helmet but clear of the eyes
		AddCap(0.38f, 0.035f, Color);
		break;
	case ECIRLItemShape::Hood:
	case ECIRLItemShape::FoolHood:
		// Comes lower still, and a little further out at the back
		AddCap(0.30f, 0.05f, Color, 1.05f, 0.04f);
		if (Item.Shape == ECIRLItemShape::FoolHood)
		{
			// Two ass's ears with bells
			for (const float Side : { -1.f, 1.f })
			{
				AddPart(EPartMesh::Sphere, FVector(-0.05f * R, Side * 0.45f * R, 1.10f * R), FVector(0.16f * R, 0.16f * R, 0.42f * R), FLinearColor(0.85f, 0.7f, 0.15f));
			}
		}
		break;
	default:
		AddCap(0.50f, 0.08f, Color);
		break;
	}
}

void UBallWornGearComponent::BuildBody(ECIRLEquipSlot Slot, const FCIRLItemRow& Item, const FLinearColor& Color)
{
	// A bowl around the lower half. The middle is set lower and the bowl made shorter and wider for each layer out,
	// so the tunic shows highest, then the padding, the mail, and the plate lowest: (middle height, half width, half height)
	struct FBowl { float CenterZ; float HalfWidth; float HalfHeight; };
	FBowl Bowl = { -0.25f, 1.03f, 0.84f };
	switch (Slot)
	{
	case ECIRLEquipSlot::Gambeson:	Bowl = { -0.35f, 1.04f, 0.74f }; break;
	case ECIRLEquipSlot::Mail:		Bowl = { -0.45f, 1.055f, 0.62f }; break;
	case ECIRLEquipSlot::Plate:		Bowl = { -0.52f, 1.07f, 0.56f }; break;
	default: break;
	}
	if (Item.Shape == ECIRLItemShape::BowlShort)
	{
		// A skirt: only the hips
		Bowl.CenterZ = -0.62f;
		Bowl.HalfHeight = 0.46f;
	}
	else if (Item.Shape == ECIRLItemShape::BowlFull)
	{
		// A full harness comes up to just under the face
		Bowl = { -0.30f, 1.075f, 0.80f };
	}
	AddShell(0.f, Bowl.CenterZ, Bowl.HalfWidth, Bowl.HalfWidth, Bowl.HalfHeight, Color);
}

void UBallWornGearComponent::BuildBeltItem(const FCIRLItemRow& Item, const FLinearColor& Color, float Side)
{
	// On the belt at the front of the hip, hanging down
	const float Angle = FMath::DegreesToRadians(55.f);
	const FVector Hang(FMath::Cos(Angle) * 1.13f * R, Side * FMath::Sin(Angle) * 1.13f * R, -0.32f * R);
	const float L = FMath::Max(Item.ReachCm, 12.f);
	switch (Item.Shape)
	{
	case ECIRLItemShape::Dagger:
		// Hilt above the belt, the blade in its sheath below
		AddPart(EPartMesh::Cylinder, Hang + FVector(0.f, 0.f, 5.f), FVector(2.6f, 2.6f, 10.f), Leather);
		AddPart(EPartMesh::Cube, Hang + FVector(0.f, 0.f, -L * 0.5f), FVector(2.f, 3.2f, L), Steel);
		break;
	case ECIRLItemShape::Purse:
		AddPart(EPartMesh::Sphere, Hang + FVector(0.f, 0.f, -9.f), FVector(11.f, 11.f, 13.f), Color);
		break;
	case ECIRLItemShape::ArrowBag:
		// A linen bag of arrows slanting back along the hip
		AddPart(EPartMesh::Cylinder, Hang + FVector(-10.f, 0.f, -20.f), FVector(11.f, 11.f, 58.f), Color, FRotator(-25.f, 0.f, 0.f));
		break;
	case ECIRLItemShape::Torch:
		AddPart(EPartMesh::Cylinder, Hang + FVector(0.f, 0.f, -L * 0.4f), FVector(3.5f, 3.5f, L), Wood);
		AddPart(EPartMesh::Sphere, Hang + FVector(0.f, 0.f, L * 0.1f + 4.f), FVector(8.f, 8.f, 11.f), FLinearColor(0.2f, 0.17f, 0.13f));
		break;
	default:
		AddPart(EPartMesh::Cube, Hang + FVector(0.f, 0.f, -8.f), FVector(6.f, 6.f, 12.f), Color);
		break;
	}
}

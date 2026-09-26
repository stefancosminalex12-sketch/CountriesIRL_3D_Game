// CountriesIRL 3D Game

#include "Characters/BallSkeletonComponent.h"
#include "Characters/BallParts.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Basic shapes are 100 cm across (and the cylinder 100 cm tall) at scale 1 */
	FVector SizeToScale(const FVector& SizeCm) { return SizeCm / 100.f; }
}

UBallSkeletonComponent::UBallSkeletonComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SphereMesh = BallParts::LoadSphere();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	CylinderMesh = Cylinder.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Eyes(TEXT("/Game/CountriesIRL/Characters/Materials/M_BallEyes.M_BallEyes"));
	EyeMaterial = Eyes.Object;
}

UStaticMeshComponent* UBallSkeletonComponent::AddPart(USceneComponent* Parent, UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor* BoneColor)
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(GetOwner());
	Part->SetStaticMesh(Mesh);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetupAttachment(Parent);
	Part->SetRelativeTransform(FTransform(Rotation, Location, Scale));
	Part->RegisterComponent();
	if (BoneColor)
	{
		BallParts::SetColor(Part, *BoneColor);
	}
	Parts.Add(Part);
	return Part;
}

void UBallSkeletonComponent::Show(USceneComponent* Parent, float GroundZ, const FLinearColor& BoneColor)
{
	if (IsShown() || !Parent)
	{
		return;
	}

	// Laid out along the ball's forward axis: skull in front, then neck, chest with ribs, pelvis.
	// Every piece rests on the ground (GroundZ + half its thickness).
	const FRotator AlongX(90.f, 0.f, 0.f);   // turns a cylinder's axis from Z to X

	// Skull: big, tipped back so the hollow sockets look at the sky
	const float SkullRadius = SkullSize * 0.5f;
	UStaticMeshComponent* Skull = AddPart(Parent, SphereMesh, FVector(40.f, 0.f, GroundZ + SkullRadius), FRotator(60.f, 0.f, 15.f), FVector(SkullSize / 100.f), &BoneColor);
	if (EyeMaterial)
	{
		// The same eye shader as living balls, in "skull" mode (Dead = 2), on a shell just above the skull
		UStaticMeshComponent* Sockets = AddPart(Skull, SphereMesh, FVector::ZeroVector, FRotator::ZeroRotator, FVector((SkullSize + 1.2f) / SkullSize), nullptr);
		Sockets->SetCastShadow(false);
		UMaterialInstanceDynamic* SocketMaterial = Sockets->CreateDynamicMaterialInstance(0, EyeMaterial);
		SocketMaterial->SetScalarParameterValue(TEXT("EyeScale"), 1.f);
		SocketMaterial->SetScalarParameterValue(TEXT("Dead"), 2.f);
	}

	// Neck: two chunky vertebrae, thick but thinner than the chest
	for (const float X : { 3.f, -8.f })
	{
		AddPart(Parent, CylinderMesh, FVector(X, 0.f, GroundZ + 8.5f), AlongX, SizeToScale(FVector(17.f, 17.f, 8.f)), &BoneColor);
	}

	// Chest: the thickest bone, the core of the skeleton
	AddPart(Parent, SphereMesh, FVector(-36.f, 0.f, GroundZ + 11.f), FRotator::ZeroRotator, SizeToScale(FVector(48.f, 25.f, 22.f)), &BoneColor);

	// Three chunky ribs across the chest, cartoon bones with knobby ends
	const float RibThickness = RibThicknessAndKnob.X;
	const float KnobSize = RibThicknessAndKnob.Y;
	const float RibX[] = { -22.f, -36.f, -50.f };
	const float RibLength[] = { 66.f, 72.f, 58.f };
	for (int32 Index = 0; Index < 3; ++Index)
	{
		AddPart(Parent, SphereMesh, FVector(RibX[Index], 0.f, GroundZ + RibThickness * 0.5f + 1.f), FRotator::ZeroRotator,
			SizeToScale(FVector(RibThickness, RibLength[Index], RibThickness)), &BoneColor);
		for (const float Side : { -1.f, 1.f })
		{
			AddPart(Parent, SphereMesh, FVector(RibX[Index], Side * (RibLength[Index] * 0.5f - 3.f), GroundZ + KnobSize * 0.5f), FRotator::ZeroRotator,
				FVector(KnobSize / 100.f), &BoneColor);
		}
	}

	// Small pelvis
	AddPart(Parent, SphereMesh, FVector(-72.f, 0.f, GroundZ + 6.f), FRotator::ZeroRotator, SizeToScale(FVector(22.f, 40.f, 12.f)), &BoneColor);
}

// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/EquipmentSlot.h"
#include "BallWornGearComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
struct FCIRLItemRow;

/**
 *  Shows what a ball wears as simple stand-in shapes that hug the sphere (Master file: worn gear is made for a
 *  ball): helmets and hats are domes on top, coifs and hoods caps that come down the back, clothing and armour
 *  rounded bands around the lower half (tunic, padding, mail, plate: each layer shows as a stripe above the next),
 *  a cloak over the back, a belt with what hangs from it, a shield on the back, a collar as a gold ring.
 *  Gloves and boots recolour the hands and boots. Shapes and colours come from the item table (Shape, Tint).
 *  Real models replace these shapes here later.
 */
UCLASS(ClassGroup=(Ball))
class UBallWornGearComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Rebuilds the shapes from what is worn now (called when the inventory changes) */
	void Refresh();

	/** First-person: the player doesn't see their own gear from the inside, but it still casts its shadow */
	void SetOwnerNoSee(bool bHide);

	/** Only bones remain: nothing is worn any more */
	void SetShown(bool bShow);

protected:

	virtual void BeginPlay() override;

private:

	enum class EPartMesh : uint8 { Sphere, Cylinder, Cube };

	/** Adds one shape on the body: centre and size (full width/depth/height) in cm, in the body's own space (X forward, Z up) */
	void AddPart(EPartMesh Mesh, const FVector& Center, const FVector& Size, const FLinearColor& Color, const FRotator& Rotation = FRotator::ZeroRotator);

	/** A dome or band: an egg shape around a point on the ball's axis, sizes as shares of the ball's radius */
	void AddShell(float CenterX, float CenterZ, float HalfLength, float HalfWidth, float HalfHeight, const FLinearColor& Color);

	/** A hat or helmet dome sitting on the ball from RimHeight up (share of the radius above the centre), always a
	 *  little wider and TopRoom taller than the ball inside it. LengthScale and ShiftBack stretch it toward the back */
	void AddCap(float RimHeight, float TopRoom, const FLinearColor& Color, float LengthScale = 1.f, float ShiftBack = 0.f);

	void BuildHead(const FCIRLItemRow& Item, const FLinearColor& Color, bool bUnderHelmet);
	void BuildBody(ECIRLEquipSlot Slot, const FCIRLItemRow& Item, const FLinearColor& Color);
	void BuildBeltItem(const FCIRLItemRow& Item, const FLinearColor& Color, float Side);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CubeMesh;

	int32 Used = 0;

	/** The ball's radius (cm): every shape is sized from it */
	float R = 68.f;

	bool bOwnerNoSee = false;
	bool bShown = true;
};

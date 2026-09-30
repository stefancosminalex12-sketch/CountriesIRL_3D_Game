// Crowns & Commoners

#include "Characters/BallParts.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace BallParts
{
	namespace
	{
		const TCHAR* ShapeMaterialPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");

		/** The engine's simple material with a "Color" parameter. Loadable at any time (runtime parts too). */
		UMaterialInterface* ShapeMaterial()
		{
			static TWeakObjectPtr<UMaterialInterface> Cached;
			if (!Cached.IsValid())
			{
				Cached = LoadObject<UMaterialInterface>(nullptr, ShapeMaterialPath);
			}
			return Cached.Get();
		}
	}

	UStaticMeshComponent* Create(AActor* Owner, FName Name, USceneComponent* Parent, UStaticMesh* Mesh)
	{
		UStaticMeshComponent* Part = Owner->CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Parent);
		Part->SetStaticMesh(Mesh);
		Part->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Part->SetGenerateOverlapEvents(false);
		Part->SetCanEverAffectNavigation(false);
		Part->CanCharacterStepUpOn = ECB_No;

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(ShapeMaterialPath);
		Part->SetMaterial(0, Material.Object);
		return Part;
	}

	void SetColor(UStaticMeshComponent* Part, const FLinearColor& Color)
	{
		if (!Part)
		{
			return;
		}

		// Parts created at runtime start with the engine's default grid material, which has no "Color"
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Part->GetMaterial(0));
		if (!Material)
		{
			Material = Part->CreateDynamicMaterialInstance(0, ShapeMaterial());
		}
		if (Material)
		{
			Material->SetVectorParameterValue(TEXT("Color"), Color);
		}
	}

	UStaticMesh* LoadSphere()
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		return Sphere.Object;
	}

	UStaticMesh* LoadCube()
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
		return Cube.Object;
	}

	UStaticMesh* LoadCylinder()
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		return Cylinder.Object;
	}
}

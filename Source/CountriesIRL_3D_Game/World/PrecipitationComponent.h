// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PrecipitationComponent.generated.h"

/**
 *  Falling rain or snow around the viewer: many small instanced meshes (streaks or flakes) in a box that
 *  follows the camera but stays anchored to the world, so walking through it feels right.
 *  How much falls follows the weather (UWeatherSubsystem); the wind slants rain and drifts snow.
 *  Simple placeholder for a Niagara effect later; cheap enough for now.
 */
UCLASS(ClassGroup=(Weather), meta=(BlueprintSpawnableComponent))
class UPrecipitationComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:

	UPrecipitationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Snowflakes instead of raindrops */
	UPROPERTY(EditAnywhere, Category="Precipitation")
	bool bSnow = false;

	/** Drops in the air at full intensity */
	UPROPERTY(EditAnywhere, Category="Precipitation")
	int32 MaxDrops = 3000;

	/** Half-size of the box of falling drops around the camera, and its height (cm) */
	UPROPERTY(EditAnywhere, Category="Precipitation")
	float AreaRadius = 1400.f;

	UPROPERTY(EditAnywhere, Category="Precipitation")
	float AreaHeight = 1600.f;

	/** Falling speed (cm/s): rain ~9 m/s, snow ~1 m/s */
	UPROPERTY(EditAnywhere, Category="Precipitation")
	float FallSpeed = 900.f;

	/** How strongly the wind pushes drops sideways (share of the wind speed) */
	UPROPERTY(EditAnywhere, Category="Precipitation")
	float WindInfluence = 0.3f;

	/** Size of each drop in cm (width, width, length) */
	UPROPERTY(EditAnywhere, Category="Precipitation")
	FVector DropSize = FVector(0.8f, 0.8f, 45.f);

private:

	float Amount = 0.f;
	float Time = 0.f;
	TArray<FTransform> Transforms;
};

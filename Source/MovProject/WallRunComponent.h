#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRunComponent.generated.h"


class APlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOVPROJECT_API UWallRunComponent : public UActorComponent
{
	GENERATED_BODY()

	APlayerCharacter* OwnerPlayerCharacter;

	UPrimitiveComponent* OwnerRootComponent;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	bool bIsWallRunning = false;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	FVector WallNormal;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	FVector WallRunDirection;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	float WallRunGravityScale;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	float WallRunSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	float WallJumpForce;

public:	
	// Sets default values for this component's properties
	UWallRunComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/** WALL RUNNING */

	void CheckForWall();

	void StartWallRun(const FVector& HitNormal);

	void StopWallRun();

	void JumpOffWall();

private:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

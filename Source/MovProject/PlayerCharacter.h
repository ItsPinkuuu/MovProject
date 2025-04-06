#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "MovProjectCharacter.h"
#include "WallRunComponent.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class MOVPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
	// First person camera
	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

	// MappingContext
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InputMappingContext;
	
	// Jump Input Action
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	// Move Input Action
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// Look Input Action
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	/** JUMP */
	
	UPROPERTY(EditAnywhere, Category = "Movement|Jump", meta = (AllowPrivateAccess = "true"))
	bool bCanDoubleJump = false;

	UPROPERTY(EditAnywhere, Category = "Movement|Jump", meta = (AllowPrivateAccess = "true"))
	bool bIsGrounded = true;

	UPROPERTY(EditAnywhere, Category = "Movement|Jump", meta = (AllowPrivateAccess = "true"))
	float CharLaunchForce;

	/** DASH */
	
	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	float DashDistance;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	float DashDuration;

	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	bool bIsDashing = false;

	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	bool bCanDash = true;

	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	float DashCooldown;

	UPROPERTY(EditAnywhere, Category = "Movement|Dash", meta = (AllowPrivateAccess = "true"))
	float DashTimeElapsed;

	/** WALL RUNNING */
	
	UPROPERTY(EditAnywhere)
	UWallRunComponent* WallRunComponent;
	
	
public:
	// Sets default values for this character's properties
	APlayerCharacter();

	

protected:

	virtual void Landed(const FHitResult& Hit) override;

	virtual void Tick(float DeltaTime) override;

	void DoubleJump();

	/** INPUT FUNCTIONS */
	// Called for movement input
	void Move(const FInputActionValue& Value);

	// Called for look input
	void Look(const FInputActionValue& Value);

	// Called for dashing
	void StartDash();

	void StopDash();

	void ResetDashCooldown();

	/** WALL RUNNING */
	FHitResult CheckWall(bool bRight);

	void ToggleWallRun(bool bEnable);
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void NotifyControllerChanged();


private:
	/** DASH */
	FTimerHandle DashCooldownTimer;
	
	FVector DashStartLocation;
	FVector DashEndLocation;
	FVector DashDirection;

	FVector2D MovementVector;

	/** WALL RUNNING */
	
};

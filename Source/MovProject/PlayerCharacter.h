#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "MovProjectCharacter.h"
#include "WallRunComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	
	// UInputAction* ForwardMoveAction;
	//
	// UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	// UInputAction* RightMoveAction;

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

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	bool bOnWall;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	bool bIsWallRunning;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	bool bIsWallRunningR;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	bool bIsWallRunningL;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	bool bWallRunGravity;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	float WallRunGravityScale;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	float WallRunSpeed;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	float WallRunJumpForce;
	
	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	float WallRunTraceRange;

	UPROPERTY(EditAnywhere, Category = "Movement|Wall Running", meta = (AllowPrivateAccess = "true"))
	bool bWallRunSuppressed;
	
	
	// UPROPERTY(EditAnywhere)
	// UWallRunComponent* WallRunComponent;

	/** SLIDING */
	
	
public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:

	virtual void Landed(const FHitResult& Hit) override;

	virtual void Tick(float DeltaTime) override;
	
	/** INPUT FUNCTIONS */
	// Called for movement input
	void Move(const FInputActionValue& Value);
	
	// Called for look input
	void Look(const FInputActionValue& Value);

	// Called for Double Jump
	void DoubleJump();

	// Called for dashing
	void StartDash();

	void StopDash();

	void ResetDashCooldown();

	// Called for Wall Running
	void WallRunUpdate(float DeltaTime);
	
	void CheckForWall();

	bool ValidWallNormal(FVector WallNormal) const;

	void PLayerStickToWall();

	void StopWallRun();

	void SuppressWallRun(float WallRunSuppressDelay);

	void ResetWallRunSuppression();

	void WallRunCameraTilt(float TargetXRoll);

	// Called for Sliding
	void StartSliding();

	void StopSliding();
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void NotifyControllerChanged();


private:
	/** DEFAULT MOVEMENT */
	FVector2D MovementVector;

	/** DASH */
	FTimerHandle DashCooldownTimer;
	
	FVector DashStartLocation;
	FVector DashEndLocation;
	FVector DashDirection;

	/** WALL RUNNING */
	FTimerHandle WallRunSuppressTimer;
	
	FVector WallRunNormal;
	float WallRunDirection;

	FVector PlayerToWallVector = (WallRunNormal - GetActorLocation()).Length() * WallRunNormal;
	FVector MovePlayerForwardVector;

	float DefaultGravity = 1.25f;

	float PlayerGravity = GetCharacterMovement()->GravityScale;
	
};

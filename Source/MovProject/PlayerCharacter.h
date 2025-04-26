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

	// Look Input Action
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlideCrouchAction;

	/** WALKING */
	UPROPERTY(EditAnywhere, Category = "Movement|Walking", meta = (AllowPrivateAccess = "true"))
	float ForwardWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement|Walking", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed;
	
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

	/** CROUCHING */

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	bool bCanStandBool;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouchKeyDown;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float StandingCameraZOffset;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float CrouchingCameraHeight;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float CrouchHeightChangeSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float StandHeightChangeSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float CrouchWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement|Crouching", meta = (AllowPrivateAccess = "true"))
	float UncrouchDelay;
	
	/** SLIDING */

	UPROPERTY(EditAnywhere, Category = "Movement|Sliding", meta = (AllowPrivateAccess = "true"))
	bool bIsSliding;

	UPROPERTY(EditAnywhere, Category = "Movement|Sliding", meta = (AllowPrivateAccess = "true"))
	float SlidingSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement|Sliding", meta = (AllowPrivateAccess = "true"))
	float SlidingDuration;

	UPROPERTY(EditAnywhere, Category = "Movement|Sliding", meta = (AllowPrivateAccess = "true"))
	float SlideFriction;
	
public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:

	virtual void Landed(const FHitResult& Hit) override;

	virtual void Tick(float DeltaTime) override;

	
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

	// Called for Crouching and Sliding

	void CrouchHeightChange(float CameraZHeight, float CapsuleHalfHeight, float HeightChangeSpeed);
	
	// Crouch
	void BeginCrouch();

	void EndCrouch();
	
	bool CanStand();

	// Sliding
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

	/** CROUCHING */
	FTimerHandle UncrouchTimerHandle;

	/** SLIDING */
	FTimerHandle SlideTimerHandle;

	FVector FloorNormal;
	
};

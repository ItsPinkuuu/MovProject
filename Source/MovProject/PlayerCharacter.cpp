#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FrameTypes.h"
#include "MovProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "VectorTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creates Camera Component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;
	
}


void APlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::DoubleJump);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		// Looking
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		// Dashing
		Input->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerCharacter::StartDash);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDashing)
	{
		DashTimeElapsed += DeltaTime;
		float Alpha = FMath::Clamp(DashTimeElapsed / DashDuration, 0.0f, 1.0f);
		FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, Alpha);
		SetActorLocation(NewLocation, true);

		if (Alpha >= 1.0f)
		{
			StopDash();
		}
	}

	/** WALL RUNNING */
	if (GetCharacterMovement()->IsFalling() && !bIsGrounded)
	{
		FHitResult HitOutR = CheckWall(true);
		FHitResult HitOutL = CheckWall(false);

		if (WallHit.bBlockingHit) PreviousWallHitNormal = WallHit.ImpactNormal;

		WallHit = HitOutR.Distance >= HitOutL.Distance ? HitOutR : HitOutL;

		if (WallHit.bBlockingHit && !bIsWallRunning)
		{
			GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, TEXT("Touching Wall"), false);
			ToggleWallRun(true);
		} else if (!WallHit.bBlockingHit && bIsWallRunning)
		{
			GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, TEXT("Not Touching Wall"), false);
			ToggleWallRun(false);
		}
	}
	else if (bIsWallRunning)
	{
		ToggleWallRun(false);
	}
}

/** DASHING */

void APlayerCharacter::StartDash()
{
	if (bIsDashing || !bCanDash) return;

	DashDirection = GetLastMovementInputVector();

	if (DashDirection.IsNearlyZero())
	{
		DashDirection = GetVelocity().GetSafeNormal();
	}

	if (DashDirection.IsNearlyZero()) return;

	if (DashDirection.Z) return;
	
	bIsDashing = true;
	bCanDash = false;
	DashTimeElapsed = 0.0f;
	
	DashStartLocation = GetActorLocation();
	DashEndLocation = DashStartLocation + DashDirection * DashDistance;

	GetWorldTimerManager().SetTimer(DashCooldownTimer, this, &APlayerCharacter::ResetDashCooldown, DashCooldown, false);
}

void APlayerCharacter::StopDash()
{
	bIsDashing = false;
}

void APlayerCharacter::ResetDashCooldown()
{
	bCanDash = true;
}

/** WALL RUNNING */
FHitResult APlayerCharacter::CheckWall(bool bRight)
{
	const FName TraceTag("WallTraceTag");
	// GetWorld()->DebugDrawTraceTag = TraceTag;

	FCollisionQueryParams TraceParams = FCollisionQueryParams(TraceTag, true, this);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bFindInitialOverlaps = true;

	FHitResult HitOut(ForceInit);
	float DirectionMultiplier = bRight ? WallRunRange : -1.0f * WallRunRange;
	FVector End = GetActorLocation() +
		(GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius()) +
			(GetActorRightVector() * DirectionMultiplier);

	FCollisionShape TraceShape = FCollisionShape::MakeSphere(30.0f);

	GetWorld()->SweepSingleByChannel(
		HitOut,
		GetActorLocation(),
		End,
		GetActorRotation().Quaternion(),
		ECollisionChannel::ECC_Visibility,
		TraceShape,
		TraceParams
		);

	return HitOut;
}

void APlayerCharacter::ToggleWallRun(bool bEnable)
{
	if (bEnable)
	{
		GetCharacterMovement()->GravityScale = WallRunGravityScale;

		APlayerController* PC = Cast<APlayerController>(GetController());
		PC->PlayerCameraManager->ViewPitchMax = 20.0f;
		PC->PlayerCameraManager->ViewPitchMin = -20.0f;

		if (FVector::DotProduct(GetActorUpVector(), GetVelocity() / GetVelocity().Size()) > 0.0f &&
			PreviousWallHitNormal != WallHit.ImpactNormal)
		{
			LaunchCharacter(GetActorUpVector() * 0, false, true);
		}

		bIsWallRunning = true;
	}
	else
	{
		GetCharacterMovement()->GravityScale = DefaultGravityScale;

		APlayerController* PC = Cast<APlayerController>(GetController());
		PC->PlayerCameraManager->ViewPitchMax = 89.900002f;
		PC->PlayerCameraManager->ViewPitchMin = -89.900002f;

		bIsWallRunning = false;
	}
}

/** DOUBLE JUMPING */
void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	bIsGrounded = true;
	bCanDoubleJump = false;
}

void APlayerCharacter::DoubleJump()
{
	if (bIsWallRunning)
	{
		FVector LaunchVector = WallHit.ImpactNormal;
		LaunchVector.Normalize();

		LaunchVector = (LaunchVector + (GetActorUpVector() * 1.0)) / 2;

		LaunchCharacter(LaunchVector * WallJumpForce, false, false);
	}
	else
	{
		if (bIsGrounded && !bCanDoubleJump)
		{
			ACharacter::Jump();
			
			bIsGrounded = false;
			bCanDoubleJump = true;
			
		} else if (!bIsGrounded && bCanDoubleJump)
		{
			LaunchCharacter(FVector(0.0f, 0.0f, CharLaunchForce), false, true);
			bCanDoubleJump = false;
		}
	}
}

/** DEFAULT MOVEMENT */

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	MovementVector = Value.Get<FVector2D>();

	if (Controller == nullptr) return;
	
	AddMovementInput(GetActorForwardVector(), MovementVector.Y);

	if (!bIsWallRunning)
	{
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

/** LOOKING AROUND */

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


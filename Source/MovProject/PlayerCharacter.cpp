#include "PlayerCharacter.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MovProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Concepts/Iterable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creates Camera Component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;

	// Creates Wall Running Component
	// WallRunComponent = CreateDefaultSubobject<UWallRunComponent>(TEXT("Wall Run Component"));
	
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
		// Input->BindAction(ForwardMoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::MoveForward);
		// Input->BindAction(RightMoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::MoveRight);
		
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

	if (!bWallRunSuppressed)
	{
		WallRunUpdate(DeltaTime);
	}
	
	if (bIsWallRunningL)
	{
		WallRunCameraTilt(15.0f);
		
	} else if (bIsWallRunningR)
	{
		WallRunCameraTilt(-15.0f);
		
	} else
	{
		WallRunCameraTilt(0.0f);
	}
	
}

/** DASHING */

void APlayerCharacter::StartDash()
{
	if (bIsDashing || !bCanDash || bIsWallRunning) return;

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
	if (bIsGrounded)
	{
		bCanDash = true;
	}
}


/** WALL RUNNING */

void APlayerCharacter::WallRunUpdate(float DeltaTime)
{
	if (MovementVector.Y < 0.5f)
	{
		return;
	}
	
	CheckForWall();
	
	if (bOnWall)
	{
		PlayerGravity = FMath::FInterpTo(PlayerGravity, WallRunGravityScale, DeltaTime, 10.0f);
		
	} else
	{
		StopWallRun();
	}
}

void APlayerCharacter::CheckForWall()
{
	FVector Forward = GetActorForwardVector();
	FVector Start = GetActorLocation();
	
	FVector EndR = Start + (GetActorRightVector() * WallRunTraceRange) + (GetActorForwardVector() * -35.0f);
	FVector EndL = Start - (GetActorRightVector() * WallRunTraceRange) + (GetActorForwardVector() * -35.0f);

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, EndR, ECC_Visibility, QueryParams))
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Trace hit");
		
		if (ValidWallNormal(Hit.ImpactNormal))
		{
			WallRunNormal = Hit.ImpactNormal;

			WallRunDirection = -1;
			PLayerStickToWall();

			bOnWall = true;
			bIsWallRunning = true;
			bIsWallRunningR = true;
			bIsWallRunningL = false;

		} else
		{
			bOnWall = false;
		}
		
	} else if (GetWorld()->LineTraceSingleByChannel(Hit, Start, EndL, ECC_Visibility, QueryParams))
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Trace hit");

		if (ValidWallNormal(Hit.ImpactNormal))
		{
			WallRunNormal = Hit.ImpactNormal;
			
			WallRunDirection = 1;
			PLayerStickToWall();

			bOnWall = true;
			bIsWallRunning = true;
			bIsWallRunningR = false;
			bIsWallRunningL = true;

		} else
		{
			bOnWall = false;
		}
		
	} else
	{
		bOnWall = false;
	}

	DrawDebugLine(GetWorld(), Start, EndR, FColor::Green, false, 0.5f);
	DrawDebugLine(GetWorld(), Start, EndL, FColor::Green, false, 0.5f);
}

bool APlayerCharacter::ValidWallNormal(FVector WallNormal) const
{
	if (WallNormal.Z >= -0.52f && WallNormal.Z <= 0.52)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, "Impact normal is VALID");

		if (GetMovementComponent()->IsFalling() == true)
		{
			// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, "Impact normal is VALID and Player is FALLING");
			return true;
		}
	}
	
	return false;
}

void APlayerCharacter::PLayerStickToWall()
{
	// Stick player to wall
	LaunchCharacter(PlayerToWallVector, false, false);

	// Move player along the wall
	LaunchCharacter(
		FVector::CrossProduct(WallRunNormal, FVector(0.0f, 0.0f, 1.0f)) * (WallRunSpeed * WallRunDirection),
		true, !bWallRunGravity);
}

void APlayerCharacter::StopWallRun()
{
	if (bIsWallRunning)
	{
		bOnWall = false;
		bIsWallRunning = false;
		bIsWallRunningR = false;
		bIsWallRunningL = false;

		PlayerGravity = DefaultGravity;
	}
}

void APlayerCharacter::SuppressWallRun(float WallRunSuppressDelay)
{
	bWallRunSuppressed = true;

	GetWorldTimerManager().SetTimer(WallRunSuppressTimer, this, &APlayerCharacter::ResetWallRunSuppression, WallRunSuppressDelay, false);
}

void APlayerCharacter::ResetWallRunSuppression()
{
	bWallRunSuppressed = false;

	GetWorldTimerManager().ClearTimer(WallRunSuppressTimer);
}

void APlayerCharacter::WallRunCameraTilt(float TargetXRoll)
{
	FRotator CurrentControllRotation = this->GetController()->GetControlRotation();
	FRotator NewXYZRotation = FRotator(CurrentControllRotation.Pitch, CurrentControllRotation.Yaw, TargetXRoll);
	FRotator NewControllRotation = FMath::RInterpTo(CurrentControllRotation, NewXYZRotation, GetWorld()->GetTimeSeconds(), 10.0f);
	
	this->GetController()->SetControlRotation(NewControllRotation);
}

/** DOUBLE JUMPING */
void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	bIsGrounded = true;
	bCanDoubleJump = false;
	bCanDash = true;

	StopWallRun();
	bWallRunSuppressed = false;
}

void APlayerCharacter::DoubleJump()
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

	// Wall Running
	if (bIsWallRunning)
	{
		bCanDoubleJump = true;
		
		SuppressWallRun(0.35f);
		
		LaunchCharacter(FVector(WallRunNormal.X * WallRunJumpForce, WallRunNormal.Y * WallRunJumpForce, CharLaunchForce), false, true);
	}
}

/** DEFAULT MOVEMENT */

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	MovementVector = Value.Get<FVector2D>();

	if (Controller == nullptr) return;
	
	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	AddMovementInput(GetActorRightVector(), MovementVector.X);
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


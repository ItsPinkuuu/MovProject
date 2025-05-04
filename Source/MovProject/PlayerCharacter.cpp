#include "PlayerCharacter.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MovProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Concepts/Iterable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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

		// Slide Crouch
		Input->BindAction(SlideCrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::BeginCrouch);
		Input->BindAction(SlideCrouchAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndCrouch);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bIsMidAir = GetCharacterMovement()->Velocity.Z != 0 ? 1 : 0;

	CheckPlayerState();
	
	FindLedge();
	
	if (!bWallRunSuppressed)
	{
		WallRunUpdate(DeltaTime);
	}

	if (bIsDashing)
	{
		DashTimeElapsed += GetWorld()->DeltaTimeSeconds;
		float Alpha = FMath::Clamp(DashTimeElapsed / DashDuration, 0.0f, 1.0f);
		FVector NewLocation = FMath::Lerp(DashStartLocation, DashEndLocation, Alpha);
		SetActorLocation(NewLocation, true);

		Camera->FieldOfView = FMath::Lerp(DashFOV, WalkingFOV, Alpha);
		
		if (Alpha >= 1.0f)
		{
			StopDash();
			CurrentState = Eps_Walking;
		}
	}
	
}



void APlayerCharacter::CheckPlayerState()
{
	
	switch (CurrentState)
	{
	case Eps_Idle:
		Camera->FieldOfView = FMath::Lerp(Camera->FieldOfView, WalkingFOV, AlphaFOV);
		CrouchHeightChange(StandingCameraZOffset, StandingCapsuleHalfHeight, StandHeightChangeSpeed);

		if (!bIsMidAir)
		{
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(IdleHeadBob, 1.f);
		}
		
		if (GetCharacterMovement()->Velocity != FVector(0, 0, 0))
		{
			CurrentState = Eps_Walking;
		}
		break;

	case Eps_Walking:

		WallRunCameraTilt(0.0f);
		CrouchHeightChange(StandingCameraZOffset, StandingCapsuleHalfHeight, StandHeightChangeSpeed);

		Camera->FieldOfView = FMath::Lerp(Camera->FieldOfView, WalkingFOV, AlphaFOV);

		if (!bIsMidAir)
		{
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(WalkingHeadBob, 1.f);
		}
		
		if (MovementVector.Y > 0.0f)
		{
			this->GetCharacterMovement()->MaxWalkSpeed = ForwardWalkSpeed;
		} else
		{
			this->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
		if (GetCharacterMovement()->Velocity.Length() < 10.f)
		{
			CurrentState = Eps_Idle;
		}
		break;

	case Eps_WallRun:
		
		Camera->FieldOfView = FMath::Lerp(Camera->FieldOfView, WallRunFOV, AlphaFOV);
		
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(WallRunningHeadBob, 1.f);
		
		if (bIsWallRunningL)
		{
			WallRunCameraTilt(WallRunCameraTiltAmount);
		
		} else if (bIsWallRunningR)
		{
			WallRunCameraTilt(-WallRunCameraTiltAmount);
		}

		if (!bOnWall)
		{
			CurrentState = Eps_Walking;
		}
		break;

	case Eps_Crouch:
		
		this->GetCharacterMovement()->MaxWalkSpeed = CrouchWalkSpeed;

		if (!bIsMidAir)
		{
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(CrouchHeadBob, 1.f);
		}
		
		if (!bIsSliding)
		{
			Camera->FieldOfView = FMath::Lerp(Camera->FieldOfView, WalkingFOV, AlphaFOV);
		}

		if (bIsCrouchKeyDown || CanStand() == false || bIsSliding)
		{
			CrouchHeightChange(CrouchingCameraHeight, CrouchingCapsuleHalfHeight, CrouchHeightChangeSpeed);
			CanStand();
		} else if (!bIsCrouchKeyDown || (CanStand() == true))
		{
			EndCrouch();
			CurrentState = Eps_Walking;
		}
		break;

	case Eps_Climbing:
		GetCharacterMovement()->Velocity = FVector(0, 0, 0);

		TimeSinceClimbStart += GetWorld()->DeltaTimeSeconds;
		if (TimeSinceClimbStart >= ClimbTime)
		{
			DontClimb();
			CurrentState = Eps_Walking;
		}
		break;
		
	default:
		UE_LOG(LogTemp, Error, TEXT("No player state found. MoodCharacter.cpp - CheckPlayerState"));
		CurrentState = Eps_Idle;
		break;
	}
}



/** DASHING */

void APlayerCharacter::StartDash()
{
	if (bIsDashing || !bCanDash || bIsWallRunning || bIsCrouching) return;

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
	this->GetMovementComponent()->Velocity.Z = 0.0f;
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
	CheckForWall();
	
	if (bOnWall)
	{
		CurrentState = Eps_WallRun;
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
	
	FVector EndSR = Start + (GetActorRightVector() * WallRunTraceRange) + (GetActorForwardVector() * -35.0f);
	FVector EndSL = Start - (GetActorRightVector() * WallRunTraceRange) + (GetActorForwardVector() * -35.0f);

	FVector EndR = Start + (GetActorRightVector() * (WallRunTraceRange));
	FVector EndL = Start - (GetActorRightVector() * (WallRunTraceRange));
	
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, EndR, ECC_Visibility, QueryParams) ||
		GetWorld()->LineTraceSingleByChannel(Hit, Start, EndSR, ECC_Visibility, QueryParams))
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Trace hit");
		
		if (ValidWallNormal(Hit.ImpactNormal))
		{
			WallRunNormal = Hit.ImpactNormal;

			WallRunDirection = -1;
			PLayerStickToWall();

			CurrentState = Eps_WallRun;
			
			bOnWall = true;
			bIsWallRunning = true;
			bIsWallRunningR = true;
			bIsWallRunningL = false;

		} else
		{
			bOnWall = false;
		}
		
	} else if (GetWorld()->LineTraceSingleByChannel(Hit, Start, EndL, ECC_Visibility, QueryParams) ||
		GetWorld()->LineTraceSingleByChannel(Hit, Start, EndSL, ECC_Visibility, QueryParams))
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Trace hit");

		if (ValidWallNormal(Hit.ImpactNormal))
		{
			WallRunNormal = Hit.ImpactNormal;
			
			WallRunDirection = 1;
			PLayerStickToWall();

			CurrentState = Eps_WallRun;
			
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

	/** DEBUG LINETRACE */
	
	// DrawDebugLine(GetWorld(), Start, EndSR, FColor::Red, false, 0.5f);
	// DrawDebugLine(GetWorld(), Start, EndSL, FColor::Red, false, 0.5f);
	//
	// DrawDebugLine(GetWorld(), Start, EndR, FColor::Green, false, 0.5f);
	// DrawDebugLine(GetWorld(), Start, EndL, FColor::Green, false, 0.5f);
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
		
		CurrentState = Eps_Walking;
	}
}

void APlayerCharacter::SuppressWallRun(float WallRunSuppressDelay)
{
	bWallRunSuppressed = true;
	bOnWall = false;

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
	FRotator NewControllRotation = FMath::RInterpTo(CurrentControllRotation, NewXYZRotation, GetWorld()->GetDeltaSeconds(), WallRunCameraTiltSpeed);
	
	this->GetController()->SetControlRotation(NewControllRotation);
}

/** CROUCHING and SLIDING */

void APlayerCharacter::CrouchHeightChange(float CameraZHeight, float CapsuleHalfHeight, float HeightChangeSpeed)
{
	
	// Camera
	FVector CurrentCameraLocation = Camera->GetRelativeLocation();
	FVector NewXYZLocation = FVector(CurrentCameraLocation.X, CurrentCameraLocation.Y, CameraZHeight);
	FVector NewCameraLocation = FMath::VInterpTo(
		CurrentCameraLocation, NewXYZLocation,
		GetWorld()->GetDeltaSeconds(), HeightChangeSpeed);

	Camera->SetRelativeLocation(NewCameraLocation);

	// Capsule
	float CurrentHalfHeight = this->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float NewHalfHeight = FMath::FInterpTo(
		CurrentHalfHeight, CapsuleHalfHeight,
		GetWorld()->GetDeltaSeconds(), HeightChangeSpeed);

	this->GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);
}

// Crouching
void APlayerCharacter::BeginCrouch()
{
	if (bIsCrouchKeyDown || bIsSliding || bIsWallRunning || bIsDashing) return;
	
	if (FVector::DotProduct(GetVelocity(), GetActorForwardVector()) > 1.0f)
	{
		StartSliding();
	}

	bIsCrouchKeyDown = true;
	bIsCrouching = true;

	CurrentState = Eps_Crouch;
	
}

void APlayerCharacter::EndCrouch()
{
	if (!bIsCrouching) return;

	bIsCrouchKeyDown = false;
	
	if (bIsSliding)
	{
		StopSliding();
	}
	
	if (CanStand() == true)
	{
		bIsCrouching = false;
	}
	
}

bool APlayerCharacter::CanStand()
{
	FVector Start = FVector(GetActorLocation().X, GetActorLocation().Y,
		GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	FVector End = Start + (FVector(0.0f, 0.0f,StandingCapsuleHalfHeight * 2));

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (bIsCrouching)
	{
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
		{
			bCanStandBool = false;
			return false;
		}
	}
	
	// DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.5f);
	bCanStandBool = true;
	return true;
}

// Sliding
void APlayerCharacter::StartSliding()
{
	if (bIsCrouching || bIsSliding || bIsWallRunning || bIsDashing || !bIsGrounded) return;
	
	bIsSliding = true;

	GetCharacterMovement()->BrakingFrictionFactor = SlideFriction;

	Camera->FieldOfView = SlidingFOV;

	FVector LaunchDirection = GetActorForwardVector();
	LaunchDirection.Z = 0.0f;
	LaunchDirection.Normalize();

	LaunchCharacter(LaunchDirection * SlidingSpeed, true, false);

	GetWorld()->GetTimerManager().SetTimer(SlideTimerHandle, this, &APlayerCharacter::StopSliding, SlidingDuration);
}

void APlayerCharacter::StopSliding()
{
	bIsSliding = false;

	GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
}

/** LEDGE CLIMBING */

void APlayerCharacter::FindLedge()
{
	if (bCanClimb) return;
	
	FHitResult BottomHitResult;
	FHitResult TopHitResult;

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	const FVector BottomTraceStart = GetActorLocation() + GetActorUpVector() * ReachLedgeLocation.Z;
	const FVector BottomTraceEnd = GetActorLocation() + GetActorForwardVector() * ReachLedgeLocation.X + GetActorUpVector() * ReachLedgeLocation.Z;

	const FVector TopTraceStart = GetActorLocation() + GetActorUpVector() * DistanceFromCapsuleMiddle;
	const FVector TopTraceEnd = GetActorLocation() + GetActorForwardVector() * LedgeCheckDistance + GetActorUpVector() * DistanceFromCapsuleMiddle;

	auto WallInFront = GetWorld()->LineTraceSingleByChannel(BottomHitResult, BottomTraceStart, BottomTraceEnd,
															ClimbableChannel, TraceParams, FCollisionResponseParams());

	auto WallAbove = GetWorld()->LineTraceSingleByChannel(TopHitResult, TopTraceStart, TopTraceEnd,
														  InterruptClimbingChannel, TraceParams,
														  FCollisionResponseParams());


	if (WallInFront && !WallAbove && GetCharacterMovement()->IsFalling())
	{
		
		// DrawDebugLine(GetWorld(), BottomTraceStart, BottomTraceEnd, FColor::Red, false, 0.5f);
		// DrawDebugLine(GetWorld(), TopTraceStart, TopTraceEnd, FColor::Yellow, false, 0.5f);
		
		AttemptClimb();
		CurrentState = Eps_Climbing;
		TimeSinceClimbStart = 0.f;

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = Owner;
		
		UKismetSystemLibrary::MoveComponentTo(
			RootComponent,
			GetActorLocation() + GetActorForwardVector() * ClimbingLocation.X + GetActorUpVector() * ClimbingLocation.Z,
			GetActorRotation(),
			true,
			true,
			ClimbTime,
			false,
			EMoveComponentAction::Move,
			LatentInfo);
	}
}

void APlayerCharacter::AttemptClimb()
{
	bCanClimb = true;
}

void APlayerCharacter::DontClimb()
{
	bCanClimb = false;
}

/** DOUBLE JUMPING */
void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(LandShake, 1.f);
	
	bIsGrounded = true;
	bCanDoubleJump = false;
	bCanDash = true;
	bCanClimb = false;

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
		bCanDash = true;
		
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



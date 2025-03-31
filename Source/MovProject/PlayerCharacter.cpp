#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FrameTypes.h"
#include "MovProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "VectorTypes.h"

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
		Input->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerCharacter::Dash);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	
}

void APlayerCharacter::Dash()
{
	FCollisionResponseParams ResponseParams;
	FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	QueryParams.AddIgnoredComponent(GetMesh());
	FHitResult Hit;
	const FVector StartTrace = Camera->GetComponentLocation();
	const FRotator CurrentRotation = Camera->GetComponentRotation();
	const FVector EndTrace = StartTrace + CurrentRotation.Vector() * DashForce;
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility, QueryParams, ResponseParams);
	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor(0, 175, 0), false, 1, 0, 1.333);

	if (!bHasDashed)
	{
		TimeElapsed += GetWorld()->DeltaTimeSeconds;
		const auto PlayerLocation = FMath::Lerp(StartTrace, EndTrace, DashDuraction * GetWorld()->DeltaTimeSeconds);
		SetActorLocation(EndTrace, true);
		// SetActorLocation(PlayerLocation);
		bHasDashed = true;
		FTimerHandle DashTimerHandle;
		GetWorldTimerManager().SetTimer(DashTimerHandle, this, &APlayerCharacter::ResetDash, DashDuraction, false);
	}
}

void APlayerCharacter::ResetDash()
{
	bHasDashed = false;
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bIsGrounded = true;
	bCanDoubleJump = false;
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
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}


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


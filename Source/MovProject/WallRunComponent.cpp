#include "WallRunComponent.h"

#include "PlayerCharacter.h"

// Sets default values for this component's properties
UWallRunComponent::UWallRunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}


// Called when the game starts
void UWallRunComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (OwnerPlayerCharacter)
	{
		OwnerRootComponent = Cast<UPrimitiveComponent>(OwnerPlayerCharacter->GetRootComponent());
	}
}

// Called every frame
void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsWallRunning && OwnerRootComponent)
	{
		OwnerRootComponent->SetPhysicsLinearVelocity(
			FVector(OwnerRootComponent->GetPhysicsLinearVelocity().X,
				OwnerRootComponent->GetPhysicsLinearVelocity().Y,
				WallRunGravityScale * -980.f));

		FVector NewVelocity = WallRunDirection * WallRunSpeed;
		OwnerRootComponent->SetPhysicsLinearVelocity(
			FVector(NewVelocity.X, NewVelocity.Y, OwnerRootComponent->GetPhysicsLinearVelocity().Z));

		CheckForWall();
	}
	
}

void UWallRunComponent::CheckForWall()
{
	if (!OwnerPlayerCharacter) return;

	FVector Start = OwnerPlayerCharacter->GetActorLocation();
	FVector RightVector = OwnerPlayerCharacter->GetActorRightVector();

	FVector EndRight = Start + (RightVector * 100.f);
	FVector EndLeft = Start - (RightVector * 100.f);

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPlayerCharacter);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, EndRight, ECC_Visibility, QueryParams))
	{
		StartWallRun(Hit.ImpactNormal);
	}
	else if (GetWorld()->LineTraceSingleByChannel(Hit, Start, EndLeft, ECC_Visibility, QueryParams))
	{
		StartWallRun(Hit.ImpactNormal);
	}
	else
	{
		StopWallRun();
	}
}

void UWallRunComponent::StartWallRun(const FVector& HitNormal)
{
	if (!OwnerPlayerCharacter || !OwnerRootComponent || bIsWallRunning) return;

	bIsWallRunning = true;
	WallNormal = HitNormal;

	WallRunDirection = FVector::CrossProduct(WallNormal, FVector::UpVector);
	if (FVector::DotProduct(WallRunDirection, OwnerPlayerCharacter->GetActorForwardVector()) < 0.f)
	{
		WallRunDirection *= -1;
	}

	OwnerRootComponent->SetEnableGravity(false);
}

void UWallRunComponent::StopWallRun()
{
	if (!bIsWallRunning || !OwnerRootComponent) return;

	bIsWallRunning = false;
	OwnerRootComponent->SetEnableGravity(true);
}

void UWallRunComponent::JumpOffWall()
{
	if (!OwnerPlayerCharacter || !OwnerRootComponent || !bIsWallRunning) return;

	StopWallRun();

	FVector JumpDirection = (WallNormal + FVector::UpVector).GetSafeNormal();
	OwnerRootComponent->AddImpulse(JumpDirection * WallJumpForce, NAME_None, true);
}

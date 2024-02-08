// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"

// Sets default values
ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;



	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->SetRelativeLocation(FVector(0, 0, 0));
	springArmComp->TargetArmLength = 0;
	//springArmComp->bUsePawnControlRotation = true;

	camComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CamComp"));
	camComp->SetupAttachment(springArmComp);
	//camComp->bUsePawnControlRotation = false;



	// ½ºÄÌ·¹Å» ¸Þ½Ã ÄÄÆ÷³ÍÆ® »ý¼º ¹× ºÎÂø
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempHandLeftMesh(
		TEXT("SkeletalMesh'/Game/MyContent/Hand_L.Hand_L'")
	);
	HandLeft = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("HandLeft"));


	HandLeft->SetSkeletalMesh(TempHandLeftMesh.Object);

	FRotator RotatorLeft(0, 0, 0);
	HandLeft->SetRelativeRotation(RotatorLeft);


	// ½ºÄÌ·¹Å» ¸Þ½Ã ÄÄÆ÷³ÍÆ® »ý¼º ¹× ºÎÂø
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempHandRightMesh(
		TEXT("SkeletalMesh'/Game/MyContent/Hand_R.Hand_R'")
	);
	HandRight = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("HandRight"));

	HandRight->SetSkeletalMesh(TempHandRightMesh.Object);

	FRotator RotatorRight(0, 0, 0);
	HandRight->SetRelativeRotation(RotatorRight);


}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACameraPawn::MouseMove()
{
	SetCursorPos(ScreenX, ScreenY);
}

void ACameraPawn::MouseClick(int status)
{
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;

	if (status == 0)
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	else if(status == 1)
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	else if(status == 2)
		input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	else if(status == 3)
		input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	
	if (status <= 3)
		SendInput(1, &input, sizeof(INPUT));
}


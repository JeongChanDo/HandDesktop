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
	/*
	HandLeft->SetRelativeLocation(FVector(200, -20, 0));
	
	FRotator RotatorLeft(180.0f, 90.0f, 0.0f);
	HandLeft->SetRelativeRotation(RotatorLeft);
	*/

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


	/*
	HandRight->SetRelativeLocation(FVector(200, 20, 0));

	FRotator RotatorRight(0, -90.0f, 180.0f);
	HandRight->SetRelativeRotation(RotatorRight);
	*/







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


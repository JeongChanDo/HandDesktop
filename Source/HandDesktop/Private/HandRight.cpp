// Fill out your copyright notice in the Description page of Project Settings.


#include "HandRight.h"

// Sets default values
AHandRight::AHandRight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ½ºÄÌ·¹Å» ¸Þ½Ã ÄÄÆ÷³ÍÆ® »ý¼º ¹× ºÎÂø
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempHandMesh(
		TEXT("SkeletalMesh'/Game/MyContent/Hand_R.Hand_R'")
	);
	HandRight = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("HandRight"));

	HandRight->SetSkeletalMesh(TempHandMesh.Object);
	HandRight->SetRelativeLocation(FVector(0, 0, 0));
}

// Called when the game starts or when spawned
void AHandRight::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHandRight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


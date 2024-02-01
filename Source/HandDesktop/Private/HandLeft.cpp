// Fill out your copyright notice in the Description page of Project Settings.


#include "HandLeft.h"

// Sets default values
AHandLeft::AHandLeft()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;



	// ½ºÄÌ·¹Å» ¸Þ½Ã ÄÄÆ÷³ÍÆ® »ý¼º ¹× ºÎÂø
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempHandMesh(
		TEXT("SkeletalMesh'/Game/MyContent/Hand_L.Hand_L'")
	);
	HandLeft = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("HandLeft"));

	HandLeft->SetSkeletalMesh(TempHandMesh.Object);
	HandLeft->SetRelativeLocation(FVector(0, 0, 0));
	//HandLeft->SetupAttachment(RootComponent);




}

// Called when the game starts or when spawned
void AHandLeft::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHandLeft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>

#include "Components/PoseableMeshComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

UCLASS()
class HANDDESKTOP_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PoseableMesh)
	UPoseableMeshComponent* HandRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PoseableMesh)
	UPoseableMeshComponent* HandLeft;


	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* springArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* camComp;

};

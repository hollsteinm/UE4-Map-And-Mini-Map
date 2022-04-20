// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneCaptureComponent2D.h"
#include "SceneCaptureComponentMap.generated.h"

/* A Scene capture component map is used to create an image and do management of map rendered objects. It also includes the math for figuring out the World to Map relationship of objects.*/
UCLASS()
class MAPPING_API USceneCaptureComponentMap : public USceneCaptureComponent2D
{
	GENERATED_BODY()
public:
	USceneCaptureComponentMap(const FObjectInitializer& ObjectInitializer);

	/*Given an Actor, return the Texture location of the Actor by projecting it to the Texture*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FVector ProjectActorLocationToTextureLocation(class AActor* Actor) const;

	/*Given a Scene Component, return the Texture location of the Scene Component by projecting it to the Texture*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FVector ProjectSceneComponentLocationToTextureLocation(USceneComponent* Component) const;

	/*Given a Location Vector, return the Texture location of the Vector by projecting it to the Texture*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FVector ProjectLocationToTextureLocation(const FVector& WorldLocation) const;

	/*Given an Actor, return the Texture location of the Actor by projecting it to the Texture as a 2D Coordinate*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FVector2D ProjectActorLocationToTextureLocation2D(class AActor* Actor) const;

	/*Given a Scene Component, return the Texture location of the Scene Component by projecting it to the Texture as a 2D Coordinate*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FVector2D ProjectSceneComponentLocationToTextureLocation2D(USceneComponent* Component) const;

	/*Given a Location Vector, return the Texture location of the Vector by projecting it to the Texture as a 2D Coordinate*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FVector2D ProjectLocationToTextureLocation2D(const FVector& WorldLocation) const;

	FORCEINLINE UMaterialInstanceDynamic* GetMaterialInstance() const { return RenderToMaterial; }

	/*One time call to move the camera to the world location, multiplied by the ClampAxis (only values of 1.0f or 0.0f are useful here)*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	void GoToWorldPosition(const FVector& WorldLocation, FVector ClampAxis = FVector(1.0f, 1.0f, 0.0f));

	/*Component Interface*/
	virtual void Activate(bool bReset) override;
	/*End Component Interface*/

protected:
	/*Get the View Projection Matrix of the Scene Capture Component*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FMatrix GetViewProjectionMatrix() const;

	/*Get the Scale of Capture to Image*/
	UFUNCTION(BlueprintCallable, Category = "SceneCaptureComponentMap")
	FVector2D GetViewToTextureScale() const;

	/*The material that we are rendering the scene capture to*/
	UPROPERTY(Transient, BlueprintReadOnly, Category="SceneCaptureComponentMap", meta=(AllowPrivateAccess="true"))
	UMaterialInstanceDynamic* RenderToMaterial;

	/*The template for the material that we will be constructing dynamically*/
	UPROPERTY(EditAnywhere, Category = "SceneCaptureComponentMap")
	UMaterialInterface* ParentMaterial;

	/*The texture parameter name of the instanced dynamic material*/
	UPROPERTY(EditDefaultsOnly, Category = "SceneCaptureComponentMap")
	FName MaterialParameterName;
};
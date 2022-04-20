// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "SlateBrush.h"
#include "SceneMapComponent.generated.h"

/**
Scene Map Component is used as the interface Component that supplies an icon and clamp configuration to the mapping 
logic in the UI. To add custom parameters for the mapping UI to use, simply create a child component and add what 
you need. Of course, you will also need to add widgets that handle the new properties.
**/
UCLASS()
class MAPPING_API USceneMapComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	USceneMapComponent(const FObjectInitializer& ObjectInitializer);

	/*Whether or not the Component is clamped to the edge of the map UI, or if it disappears when out of view*/
	UFUNCTION(BlueprintCallable, Category = "SceneMapComponent")
	bool ClampToMapEdge() const;

	/*The Icon to use in the Map*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SceneMapComponent")
	FSlateBrush MapIcon;

protected:
	/*Internal overridable implementation of ClampToMapEdge*/
	UFUNCTION(BlueprintNativeEvent, Category = "SceneMapComponent")
	bool ClampToMapEdgeInternal() const;

};
// Fill out your copyright notice in the Description page of Project Settings.

#include "MappingPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "Widgets/MapWidgetStyle.h"
#include "SceneMapComponent.h"

USceneMapComponent::USceneMapComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MapIcon = FMapStyle::GetDefault().ComponentBrush;
}

bool USceneMapComponent::ClampToMapEdge() const
{
	return ClampToMapEdgeInternal();
}

bool USceneMapComponent::ClampToMapEdgeInternal_Implementation() const
{
	return false;
}
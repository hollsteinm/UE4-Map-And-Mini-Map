// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Widgets/SCompoundWidget.h"
#include "SlateDelegates.h"
#include "SceneCaptureComponentMap.h"


class MAPPING_API SMap : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMap)
		: _CaptureComponent(nullptr)
	{}
	SLATE_ARGUMENT(USceneCaptureComponentMap*, CaptureComponent)
	SLATE_END_ARGS()

		/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual ~SMap();

	void SetCaptureComponent(USceneCaptureComponentMap* NewMapCaptureComponent);
	void Add(USceneMapComponent* Component);
	void Remove(USceneMapComponent* Component);
	void RemoveAll();
	void SetAll(const TArray<USceneMapComponent*>& NewSceneComponents);

	virtual FVector2D ComputeDesiredSize(float) const override;

protected:
	virtual TSharedRef<SWidget> OnGenerateChildIcon(USceneMapComponent* Component, USceneCaptureComponentMap* CurrentMap) const;

	//Helper Functions
	FVector2D WorldLocationToMap(const FVector& WorldLocation) const;
	TAttribute<FVector2D> CreateComponentToMapPositionAttribute(USceneMapComponent* Component) const;
	EVisibility GetComponentVisibility(USceneMapComponent* Component) const;

private:
	void RemoveAllWithSlack(int32 Slack);

	//Slate Objects
	FSlateBrush MapBrush;
	TSharedPtr<SImage> RenderImage;
	TSharedPtr<SCanvas> Canvas;
	SCanvas::FSlot* MapSlot;

	//World Objects
	TWeakObjectPtr<USceneCaptureComponentMap> Map;
	TMap<TWeakObjectPtr<USceneMapComponent>, TSharedRef<SWidget>> MapIcons;
};
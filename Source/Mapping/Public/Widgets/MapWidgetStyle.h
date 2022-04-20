// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SlateCore.h"
#include "SlateBasics.h"
#include "Styling/SlateWidgetStyle.h"
#include "SlateWidgetStyleContainerBase.h"

#include "MapWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct MAPPING_API FMapStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FMapStyle();
	virtual ~FMapStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FMapStyle& GetDefault();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSlateBrush BackgroundImage;
	FMapStyle& SetBackgroundImage(const FSlateBrush& NewBackgroundImage)
	{
		BackgroundImage = NewBackgroundImage;
		return *this;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSlateBrush ComponentBrush;
	FMapStyle& SetComponentBrush(const FSlateBrush& NewComponentBrush)
	{
		ComponentBrush = NewComponentBrush;
		return *this;
	}
};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UMapWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FMapStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &WidgetStyle );
	}
};

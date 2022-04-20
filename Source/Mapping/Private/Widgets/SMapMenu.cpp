// Fill out your copyright notice in the Description page of Project Settings.

#include "MappingPrivatePCH.h"
#include "SMapMenu.h"
#include "SlateOptMacros.h"
#include "SlateBasics.h"
#include "SlateCore.h"
#include "Widgets/SPanZoomPanel.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "SceneMapComponent.h"

SMapMenu::~SMapMenu()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMapMenu::Construct(const FArguments& InArgs)
{
	HeaderVisibility = InArgs._HeaderVisibility;
	FooterVisibility = InArgs._FooterVisibility;
	LeftSidebarVisibility = InArgs._LeftSidebarVisibility;
	RightSidebarVisibility = InArgs._RightSidebarVisibility;
	

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.AutoHeight()
		[
			SNew(SBorder)
			.Visibility(HeaderVisibility)
			.Content()
			[
				InArgs._HeaderContent.Widget
			]
		]
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SBorder)
				.Visibility(LeftSidebarVisibility)
				.Content()
				[
					InArgs._LeftSidebarContent.Widget
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(MapPanel, SPanZoomPanel)
				.MapStyle(InArgs._MapStyle)
				+ SPanZoomPanel::Slot()
				.Position(FVector2D(0.0f, 0.0f))
				.Order(0)
				[
					SAssignNew(Map, SMap)
					.CaptureComponent(InArgs._MapCaptureComponent)
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SBorder)
				.Visibility(RightSidebarVisibility)
				.Content()
				[
					InArgs._RightSidebarContent.Widget
				]
			]
		]
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		.AutoHeight()
		[
			SNew(SBorder)
			.Visibility(FooterVisibility)
			.Content()
			[
				InArgs._FooterContent.Widget
			]
		]
	];	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

float SMapMenu::GetZoom() const
{
	if (MapPanel.IsValid())
	{
		return MapPanel->GetZoom();
	}
	else
	{
		return 0.0f;
	}
}

void SMapMenu::Pan(const FVector2D& PanAmountAndDirection)
{
	if (MapPanel.IsValid())
	{
		MapPanel->Pan(PanAmountAndDirection);
	}
}

void SMapMenu::PanTo(const FVector2D& DesiredViewPosition)
{
	if (MapPanel.IsValid())
	{
		MapPanel->PanTo(DesiredViewPosition);
	}
}

void SMapMenu::Zoom(float ZoomAmount)
{
	if (MapPanel.IsValid())
	{
		MapPanel->Zoom(ZoomAmount);
	}
}

void SMapMenu::SnapToZoom(float ZoomValue)
{
	if (MapPanel.IsValid())
	{
		MapPanel->SnapToZoom(ZoomValue);
	}
}

void SMapMenu::SnapToViewPosition(const FVector2D& ViewPosition)
{
	if (MapPanel.IsValid())
	{
		MapPanel->SnapToViewPosition(ViewPosition);
	}
}

void SMapMenu::SetZoomSpeed(float NewSpeed)
{
	if (MapPanel.IsValid())
	{
		MapPanel->SetZoomSpeed(NewSpeed);
	}
}

void SMapMenu::SetPanSpeed(float NewSpeed)
{
	if (MapPanel.IsValid())
	{
		MapPanel->SetPanSpeed(NewSpeed);
	}
}

void SMapMenu::SetZoomRanges(float Min, float Max, const TArray<float>& Ranges)
{
	if (MapPanel.IsValid())
	{
		MapPanel->SetZoomRanges(Min, Max, Ranges);
	}
}

SPanZoomPanel::FSlot& SMapMenu::AddSlot()
{
	return MapPanel->AddSlot();
}

void SMapMenu::RemoveSlot(TSharedRef<SWidget> Widget)
{
	if (MapPanel.IsValid())
	{
		MapPanel->RemoveSlot(Widget);
	}
}

void SMapMenu::SetHeaderVisibility(TAttribute<EVisibility> NewVisibility)
{
	HeaderVisibility = NewVisibility;
	Invalidate(EInvalidateWidget::Layout);
}

void SMapMenu::SetFooterVisibility(TAttribute<EVisibility> NewVisibility)
{
	FooterVisibility = NewVisibility;
	Invalidate(EInvalidateWidget::Layout);
}

void SMapMenu::SetLeftSidebarVisibility(TAttribute<EVisibility> NewVisibility)
{
	LeftSidebarVisibility = NewVisibility;
	Invalidate(EInvalidateWidget::Layout);
}

void SMapMenu::SetRightSidebarVisibility(TAttribute<EVisibility> NewVisibility)
{
	RightSidebarVisibility = NewVisibility;
	Invalidate(EInvalidateWidget::Layout);
}

void SMapMenu::SetCaptureComponent(USceneCaptureComponentMap* NewMapCaptureComponent)
{
	Map->SetCaptureComponent(NewMapCaptureComponent);
}

void SMapMenu::Add(USceneMapComponent* Component)
{
	Map->Add(Component);
}

void SMapMenu::Remove(USceneMapComponent* Component)
{
	Map->Remove(Component);
}

void SMapMenu::RemoveAll()
{
	Map->RemoveAll();
}

void SMapMenu::SetAll(const TArray<USceneMapComponent*>& NewSceneComponents)
{
	Map->SetAll(NewSceneComponents);
}
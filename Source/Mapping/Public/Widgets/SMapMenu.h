// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Widgets/SCompoundWidget.h"
#include "SlateDelegates.h"
#include "Widgets/MapWidgetStyle.h"
#include "SPanZoomPanel.h"
#include "SMap.h"

/**
 * 
 */
class MAPPING_API SMapMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMapMenu)
		: _HeaderContent()
		, _FooterContent()
		, _LeftSidebarContent()
		, _RightSidebarContent()
		, _MapCaptureComponent(nullptr)
		, _MapStyle(&FMapStyle::GetDefault())
		, _HeaderVisibility(EVisibility::Collapsed)
		, _FooterVisibility(EVisibility::Collapsed)
		, _LeftSidebarVisibility(EVisibility::Collapsed)
		, _RightSidebarVisibility(EVisibility::Collapsed)
	{}
		SLATE_ARGUMENT(USceneCaptureComponentMap*, MapCaptureComponent)
		SLATE_STYLE_ARGUMENT(FMapStyle, MapStyle)
		SLATE_NAMED_SLOT(FArguments, HeaderContent)
		SLATE_NAMED_SLOT(FArguments, FooterContent)
		SLATE_NAMED_SLOT(FArguments, LeftSidebarContent)
		SLATE_NAMED_SLOT(FArguments, RightSidebarContent)
		SLATE_ATTRIBUTE(EVisibility, HeaderVisibility)
		SLATE_ATTRIBUTE(EVisibility, FooterVisibility)
		SLATE_ATTRIBUTE(EVisibility, LeftSidebarVisibility)
		SLATE_ATTRIBUTE(EVisibility, RightSidebarVisibility)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual ~SMapMenu();

	/**Beg SPanZoom Wrapper**/
	float GetZoom() const;
	void Pan(const FVector2D& PanAmountAndDirection);
	void PanTo(const FVector2D& DesiredViewPosition);
	void Zoom(float ZoomAmount);
	void SnapToZoom(float ZoomValue);
	void SnapToViewPosition(const FVector2D& ViewPosition);
	void SetZoomSpeed(float NewSpeed);
	void SetPanSpeed(float NewSpeed);
	void SetZoomRanges(float Min, float Max, const TArray<float>& Ranges);
	SPanZoomPanel::FSlot& AddSlot();
	void RemoveSlot(TSharedRef<SWidget> Widget);
	/**End SpanZoom Wrapper**/

	/**Beg SMap Wrapper**/
	void SetCaptureComponent(class USceneCaptureComponentMap* NewMapcaptureComponent);
	void Add(class USceneMapComponent* Component);
	void Remove(class USceneMapComponent* Component);
	void RemoveAll();
	void SetAll(const TArray<USceneMapComponent*>& NewSceneComponents);
	/**End SMap Wrapper**/

	void SetHeaderVisibility(TAttribute<EVisibility> NewVisibility);
	void SetFooterVisibility(TAttribute<EVisibility> NewVisibility);
	void SetLeftSidebarVisibility(TAttribute<EVisibility> NewVisibility);
	void SetRightSidebarVisibility(TAttribute<EVisibility> NewVisibility);
protected:
	TSharedPtr<SPanZoomPanel> MapPanel;
	TSharedPtr<SMap> Map;

	TAttribute<EVisibility> HeaderVisibility;
	TAttribute<EVisibility> FooterVisibility;
	TAttribute<EVisibility> LeftSidebarVisibility;
	TAttribute<EVisibility> RightSidebarVisibility;
};

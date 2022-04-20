// © 2016 Martin Hollstein, All Rights Reserved.

#pragma once


#include "SPanel.h"
#include "MappingTypes.h"
#include "MapWidgetStyle.h"
#include "SlateDelegates.h"

class MAPPING_API SPanZoomPanel : public SPanel
{
public:
	virtual ~SPanZoomPanel();

	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot& Position(const TAttribute<FVector2D>& InPosition)
		{
			SlotPosition = InPosition;
			return *this;
		}

		FSlot& Scale(const TAttribute<FVector2D>& InScale)
		{
			SlotLocalScale = InScale;
			return *this;
		}

		FSlot& HAlign(EHorizontalAlignment InHAlignment)
		{
			HAlignment = InHAlignment;
			return *this;
		}

		FSlot& VAlign(EVerticalAlignment InVAlignment)
		{
			VAlignment = InVAlignment;
			return *this;
		}

		FSlot& Order(int32 InDepth)
		{
			RenderOrder = InDepth;
			return *this;
		}

		FSlot& ClampToBounds(bool bInClampToBounds)
		{
			bClampToBounds = bInClampToBounds;
			return *this;
		}

		EHorizontalAlignment HAlignment;

		EVerticalAlignment VAlignment;

		/** Default values for a slot. */
		FSlot()
			: TSlotBase<FSlot>()
			, SlotPosition(FVector2D::ZeroVector)
			, SlotLocalScale(FVector2D(1.0f, 1.0f))
			, HAlignment(HAlign_Left)
			, VAlignment(VAlign_Top)
			, RenderOrder(1)
		{ }

		bool operator<(const FSlot& RHS) const
		{
			return RenderOrder < RHS.RenderOrder;
		}

		static bool Sort(const FSlot& LHS, const FSlot& RHS)
		{
			return LHS < RHS;
		}

		FORCEINLINE FVector2D GetSlotPosition() const { return SlotPosition.Get(FVector2D::ZeroVector); }

		FORCEINLINE FVector2D GetSlotLocalScale() const { return SlotLocalScale.Get(FVector2D(1.0f, 1.0f)); }

		FORCEINLINE bool GetClampedToBounds() const { return bClampToBounds; }		

	private:
		TAttribute<FVector2D> SlotPosition;
		TAttribute<FVector2D> SlotLocalScale;
		bool bClampToBounds;
		int32 RenderOrder;
	};

	/** Slots and Slates**/
	SLATE_BEGIN_ARGS(SPanZoomPanel)
		: _MapStyle(&FMapStyle::GetDefault())
		, _MinimumDesiredSize(FVector2D::ZeroVector)
	{
	}
		SLATE_ARGUMENT(FVector2D, MinimumDesiredSize)
		SLATE_STYLE_ARGUMENT(FMapStyle, MapStyle)
		SLATE_SUPPORTS_SLOT(SPanZoomPanel::FSlot)

	SLATE_END_ARGS()

	SPanZoomPanel();

	void Construct(const FArguments& InArgs);

	static FSlot& Slot()
	{
		return *(new FSlot());
	}

	FSlot& AddSlot()
	{
		SPanZoomPanel::FSlot& NewSlot = *new FSlot();
		Children.Add(&NewSlot);
		Children.Sort(&FSlot::Sort);
		return NewSlot;
	}
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);
	void ClearChildren();
	/** End Slots and Slate**/

	/**Beg Widget Interface**/
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& GestureEvent) override;
	virtual FReply OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) override;
	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) override;
	virtual FReply OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) override;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	virtual FReply OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) override;

	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual bool SupportsKeyboardFocus() const override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	/**End Widget Interface**/

	/**Beg Panel Interface**/
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual float GetRelativeLayoutScale(const FSlotBase& Child) const override;
	/**End Panel Interface**/

	/**Pan and Zoom interface**/

	/* View methods*/
	float GetZoom() const;
	FVector2D GetViewOffset() const;
	FVector2D GetViewCenter() const;
	FVector2D GetWidgetCenter() const;

	virtual FVector2D ToWidgetPosition(const FVector2D& ViewPosition) const;
	FVector2D ToViewPosition(const FVector2D& WidgetPosition) const;

	float AngleToViewCenter(const FVector2D& Position, bool bIsInViewSpace) const;

	/*Mutators*/
	void Pan(const FVector2D& PanAmountAndDirection);
	void PanTo(const FVector2D& DesiredViewPosition);
	void Zoom(float ZoomAmount);
	void SnapToZoom(float ZoomValue);
	void SnapToViewPosition(const FVector2D& ViewPosition);

	/*Configuration*/
	void SetZoomSpeed(float NewSpeed);
	void SetPanSpeed(float NewSpeed);
	void SetZoomRanges(float Min, float Max, const TArray<float>& Ranges);

protected:
	FGeometry LastTickGeometry;

	/*Interaction*/

	//MethodName = {Query}_{HandlerContext}

	virtual bool IsPanAction_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsZoomAction_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsClickAction_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;

	virtual bool IsPanAction_OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsZoomAction_OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsClickAction_OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;

	virtual bool IsPanAction_OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsZoomAction_OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsClickAction_OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;

	virtual bool IsPanAction_OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsZoomAction_OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	virtual bool IsClickAction_OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;

	virtual bool IsPanAction_OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsZoomAction_OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsClickAction_OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;

	virtual bool IsPanAction_OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsZoomAction_OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsClickAction_OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;

	virtual bool IsPanAction_OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsZoomAction_OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsClickAction_OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;

	virtual bool IsPanAction_OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsZoomAction_OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;
	virtual bool IsClickAction_OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const;

	virtual bool IsPanAction_OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const;
	virtual bool IsZoomAction_OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const;
	virtual bool IsClickAction_OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const;

	virtual bool IsPanAction_OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const;
	virtual bool IsZoomAction_OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const;
	virtual bool IsClickAction_OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const;

	virtual bool IsPanAction_OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) const;
	virtual bool IsZoomAction_OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) const;
	virtual bool IsClickAction_OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) const;

	virtual void HandlePan(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	virtual void HandleZoom(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	virtual void HandleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	virtual void HandlePan(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent);
	virtual void HandleZoom(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent);
	virtual void HandleClick(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent);
	virtual void HandlePan(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent);
	virtual void HandleZoom(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent);
	virtual void HandleClick(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent);

	/*View methods*/
	FSlateRect GetViewRect() const;
	FSlateRect GetWidgetRect() const;

	/*Painting*/
	virtual int32 PaintBackgroundImage(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;
	virtual int32 PaintChildren(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;

	/**End Pan and Zoom interface**/

private:

	bool bIsPanning;
	bool bIsZooming;
	bool bIsClicking;

	struct
	{
		float CurrentZoom;
		float TargetZoom;
		float ZoomSpeed;
		float Min;
		float Max;
		TArray<float> ZoomValues;

		FORCEINLINE float ZoomToScale() const 
		{ 
			if (TargetZoom <= Min) return Min;
			for (auto Value : ZoomValues)
			{
				if (TargetZoom <= Value) return Value;
			}
			return Max;
		}

		FORCEINLINE void UpdateZoom(const float DeltaTime)
		{
			CurrentZoom = FMath::FInterpTo(CurrentZoom, ZoomToScale(), DeltaTime, ZoomSpeed);
		}

		void Reset()
		{
			CurrentZoom = TargetZoom = 2.0f;
			ZoomSpeed = 0.5f;
			Min = 0.5f;
			ZoomValues = { 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f };
			Max = 5.0f;
		}

		void AddZoom(float Value)
		{
			TargetZoom = FMath::Clamp(TargetZoom + Value, Min, Max);
		}

		void SetZoom(float Value)
		{
			TargetZoom = FMath::Clamp(Value, Min, Max);
		}
	} ZoomDriver;

	struct
	{
		FVector2D CurrentViewOffset;
		FVector2D TargetViewOffset;
		float PanSpeed;

		FORCEINLINE float UpdatePan(const float DeltaTime)
		{
			CurrentViewOffset = FMath::Vector2DInterpTo(CurrentViewOffset, TargetViewOffset, DeltaTime, PanSpeed);
		}

		void Reset()
		{
			CurrentViewOffset = TargetViewOffset = FVector2D::ZeroVector;
			PanSpeed = 0.333f;
		}
	} PanDriver;

	TPanelChildren<FSlot> Children;
	const FMapStyle* MapStyle;
	FVector2D MinimumDesiredSize;
};
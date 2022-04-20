// © 2016 Martin Hollstein, All Rights Reserved.

#pragma once

#include "MappingPrivatePCH.h"
#include "Widgets/SPanZoomPanel.h"
#include "SlateCore.h"
#include "IPluginManager.h"

SPanZoomPanel::~SPanZoomPanel()
{

}

SPanZoomPanel::SPanZoomPanel()
	: Children()
	, MapStyle(nullptr)
	, bIsPanning(false)
	, bIsClicking(false)
	, bIsZooming(false)
{
	ZoomDriver.Reset();
	PanDriver.Reset();
}

void SPanZoomPanel::Construct(const SPanZoomPanel::FArguments& InArgs)
{
	MapStyle = InArgs._MapStyle;
	MinimumDesiredSize = InArgs._MinimumDesiredSize;

	ZoomDriver.Reset();
	PanDriver.Reset();

	const int32 NumSlots = InArgs.Slots.Num();
	for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
	{
		Children.Add(InArgs.Slots[SlotIndex]);
	}

	Children.Sort(&FSlot::Sort);
}

void SPanZoomPanel::ClearChildren()
{
	Children.Empty();
}

int32 SPanZoomPanel::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	for (int32 SlotIdx = 0; SlotIdx < Children.Num(); ++SlotIdx)
	{
		if (SlotWidget == Children[SlotIdx].GetWidget())
		{
			Children.RemoveAt(SlotIdx);
			return SlotIdx;
		}
	}
	Children.Sort(&FSlot::Sort);
	return INDEX_NONE;
}

void SPanZoomPanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	if (Children.Num() > 0)
	{
		for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			const SPanZoomPanel::FSlot& CurChild = Children[ChildIndex];
			const FVector2D Size = CurChild.GetWidget()->GetDesiredSize();

			FVector2D Offset(0.0f, 0.0f);

			switch (CurChild.HAlignment)
			{
			case HAlign_Center:
				Offset.X = -Size.X / 2.0f;
				break;
			case HAlign_Right:
				Offset.X = -Size.X;
				break;
			case HAlign_Fill:
			case HAlign_Left:
				break;
			}

			switch (CurChild.VAlignment)
			{
			case VAlign_Bottom:
				Offset.Y = -Size.Y;
				break;
			case VAlign_Center:
				Offset.Y = -Size.Y / 2.0f;
				break;
			case VAlign_Top:
			case VAlign_Fill:
				break;
			}
			
			ArrangedChildren.AddWidget(
				AllottedGeometry.MakeChild(
					CurChild.GetWidget(),
					CurChild.GetSlotPosition() + Offset - GetViewOffset(),
					Size,
					GetZoom()
				)
			);
		}
	}
}

int32 SPanZoomPanel::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = PaintBackgroundImage(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	LayerId = PaintChildren(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	return LayerId;
}

int32 SPanZoomPanel::PaintBackgroundImage(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (MapStyle)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			&MapStyle->BackgroundImage,
			MyClippingRect
		);
	}
	return LayerId + 1;
}

int32 SPanZoomPanel::PaintChildren(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	this->ArrangeChildren(AllottedGeometry, ArrangedChildren);

	int32 MaxLayerId = LayerId;
	const FPaintArgs NewArgs = Args.WithNewParent(this);

	for (int32 ChildIndex = 0; ChildIndex < ArrangedChildren.Num(); ++ChildIndex)
	{
		FArrangedWidget& CurWidget = ArrangedChildren[ChildIndex];

		bool bWereOverlapping;
		FSlateRect ChildClipRect = MyClippingRect.IntersectionWith(CurWidget.Geometry.GetClippingRect(), bWereOverlapping);

		if (bWereOverlapping)
		{
			const int32 CurWidgetsMaxLayerId = CurWidget.Widget->Paint(NewArgs, CurWidget.Geometry, ChildClipRect, OutDrawElements, MaxLayerId + 1, InWidgetStyle, ShouldBeEnabled(bParentEnabled));
			MaxLayerId = FMath::Max(MaxLayerId, CurWidgetsMaxLayerId);
		}
	}

	return MaxLayerId;
}

float SPanZoomPanel::GetRelativeLayoutScale(const FSlotBase& Child) const
{
	return GetZoom();
}

FVector2D SPanZoomPanel::ComputeDesiredSize(float) const
{
	if (MapStyle && MinimumDesiredSize.IsNearlyZero())
	{
		return MapStyle->BackgroundImage.ImageSize;
	}
	else
	{
		return MinimumDesiredSize;
	}
}

FChildren* SPanZoomPanel::GetChildren()
{
	return &Children;
}

void SPanZoomPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	LastTickGeometry = AllottedGeometry;
	ZoomDriver.UpdateZoom(InDeltaTime);
	PanDriver.UpdatePan(InDeltaTime);
	SPanel::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

bool SPanZoomPanel::IsPanAction_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

bool SPanZoomPanel::IsZoomAction_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton;
}

bool SPanZoomPanel::IsClickAction_OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

bool SPanZoomPanel::IsPanAction_OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bIsPanning;
}

bool SPanZoomPanel::IsZoomAction_OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton && bIsZooming;
}

bool SPanZoomPanel::IsClickAction_OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsClicking;
}

bool SPanZoomPanel::IsPanAction_OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton) && bIsPanning;
}

bool SPanZoomPanel::IsZoomAction_OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton) && bIsZooming;
}

bool SPanZoomPanel::IsClickAction_OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && bIsClicking;
}

bool SPanZoomPanel::IsPanAction_OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return false;
}

bool SPanZoomPanel::IsZoomAction_OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return true;
}

bool SPanZoomPanel::IsClickAction_OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return false;
}

bool SPanZoomPanel::IsPanAction_OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return TouchEvent.GetGestureType() == EGestureEvent::Scroll;
}

bool SPanZoomPanel::IsZoomAction_OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return TouchEvent.GetGestureType() == EGestureEvent::Magnify;
}

bool SPanZoomPanel::IsClickAction_OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return false;
}

bool SPanZoomPanel::IsPanAction_OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return false;
}

bool SPanZoomPanel::IsZoomAction_OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return false;
}

bool SPanZoomPanel::IsClickAction_OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return (TouchEvent.GetGestureType() == EGestureEvent::Count || TouchEvent.GetGestureType() == EGestureEvent::None) && TouchEvent.GetPointerIndex() == 1;
}

bool SPanZoomPanel::IsPanAction_OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return bIsPanning;
}

bool SPanZoomPanel::IsZoomAction_OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return bIsZooming;
}

bool SPanZoomPanel::IsClickAction_OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return bIsClicking;
}

bool SPanZoomPanel::IsPanAction_OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return false;
}

bool SPanZoomPanel::IsZoomAction_OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return false;
}

bool SPanZoomPanel::IsClickAction_OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) const
{
	return false;
}

bool SPanZoomPanel::IsPanAction_OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const
{
	return bIsPanning &&
		(
			(
				KeyEvent.GetKey() == EKeys::NumPadFour ||
				KeyEvent.GetKey() == EKeys::NumPadEight ||
				KeyEvent.GetKey() == EKeys::NumPadTwo ||
				KeyEvent.GetKey() == EKeys::NumPadSix
			)
			||
			(
				KeyEvent.GetKey() == EKeys::Gamepad_DPad_Up ||
				KeyEvent.GetKey() == EKeys::Gamepad_DPad_Down ||
				KeyEvent.GetKey() == EKeys::Gamepad_DPad_Left ||
				KeyEvent.GetKey() == EKeys::Gamepad_DPad_Right
			)
		);
}

bool SPanZoomPanel::IsZoomAction_OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const
{
	return bIsZooming &&
		(
			(
				KeyEvent.GetKey() == EKeys::PageUp ||
				KeyEvent.GetKey() == EKeys::PageDown
			)
			||
			(
				KeyEvent.GetKey() == EKeys::Gamepad_LeftTrigger ||
				KeyEvent.GetKey() == EKeys::Gamepad_RightTrigger
			)
		);
}

bool SPanZoomPanel::IsClickAction_OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const
{
	return bIsClicking &&
		(
			KeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Bottom
		);
}

bool SPanZoomPanel::IsPanAction_OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const
{
	return
		(
			KeyEvent.GetKey() == EKeys::NumPadFour ||
			KeyEvent.GetKey() == EKeys::NumPadEight ||
			KeyEvent.GetKey() == EKeys::NumPadTwo ||
			KeyEvent.GetKey() == EKeys::NumPadSix
		)
		||
		(
			KeyEvent.GetKey() == EKeys::Gamepad_DPad_Up ||
			KeyEvent.GetKey() == EKeys::Gamepad_DPad_Down ||
			KeyEvent.GetKey() == EKeys::Gamepad_DPad_Left ||
			KeyEvent.GetKey() == EKeys::Gamepad_DPad_Right
		);
}

bool SPanZoomPanel::IsZoomAction_OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const
{
	return
		(
			KeyEvent.GetKey() == EKeys::PageUp ||
			KeyEvent.GetKey() == EKeys::PageDown
		)
		||
		(
			KeyEvent.GetKey() == EKeys::Gamepad_LeftTrigger ||
			KeyEvent.GetKey() == EKeys::Gamepad_RightTrigger
		);
}

bool SPanZoomPanel::IsClickAction_OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) const
{
	return KeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Bottom;
}

bool SPanZoomPanel::IsPanAction_OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) const
{
	return InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Down ||
		InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Up ||
		InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Left ||
		InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Right;
}

bool SPanZoomPanel::IsZoomAction_OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) const
{
	return InAnalogInputEvent.GetKey() == EKeys::Gamepad_RightStick_Down || 
		InAnalogInputEvent.GetKey() == EKeys::Gamepad_RightStick_Up;
}

bool SPanZoomPanel::IsClickAction_OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) const
{
	return false;
}

void SPanZoomPanel::HandlePan(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Pan(MouseEvent.GetCursorDelta());
	Pan(MouseEvent.GetGestureDelta());
}

void SPanZoomPanel::HandleZoom(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton))
	{
		Zoom(MouseEvent.GetCursorDelta().Y);
	}
	else
	{
		Zoom(MouseEvent.GetWheelDelta());
		Zoom(MouseEvent.GetGestureDelta().Y);
	}
}

void SPanZoomPanel::HandleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	
}

void SPanZoomPanel::HandlePan(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	if (KeyEvent.GetKey() == EKeys::Gamepad_DPad_Up || KeyEvent.GetKey() == EKeys::NumPadEight)
	{
		Pan(FVector2D(0.0f, -1.0f));
	}
	if (KeyEvent.GetKey() == EKeys::Gamepad_DPad_Down || KeyEvent.GetKey() == EKeys::NumPadTwo)
	{
		Pan(FVector2D(0.0f, 1.0f));
	}
	if (KeyEvent.GetKey() == EKeys::Gamepad_DPad_Left || KeyEvent.GetKey() == EKeys::NumPadFour)
	{
		Pan(FVector2D(-1.0f, 0.0f));
	}
	if (KeyEvent.GetKey() == EKeys::Gamepad_DPad_Right || KeyEvent.GetKey() == EKeys::NumPadSix)
	{
		Pan(FVector2D(1.0f, 0.0f));
	}
}

void SPanZoomPanel::HandleZoom(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	if (KeyEvent.GetKey() == EKeys::Gamepad_LeftTrigger || KeyEvent.GetKey() == EKeys::PageUp)
	{
		Zoom(1.0f);
	}
	if(KeyEvent.GetKey() == EKeys::PageDown || KeyEvent.GetKey() == EKeys::Gamepad_RightTrigger)
	{
		Zoom(-1.0f);
	}
}

void SPanZoomPanel::HandleClick(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	
}

void SPanZoomPanel::HandlePan(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Up)
	{
		Pan(FVector2D(0.0f, -InAnalogInputEvent.GetAnalogValue()));
	}
	if (InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Down)
	{
		Pan(FVector2D(0.0f, InAnalogInputEvent.GetAnalogValue()));
	}
	if (InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Left)
	{
		Pan(FVector2D(-InAnalogInputEvent.GetAnalogValue(), 0.0f));
	}
	if (InAnalogInputEvent.GetKey() == EKeys::Gamepad_DPad_Right)
	{
		Pan(FVector2D(InAnalogInputEvent.GetAnalogValue(), 0.0f));
	}
}

void SPanZoomPanel::HandleZoom(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent)
{
	Zoom(InAnalogInputEvent.GetAnalogValue() * (InAnalogInputEvent.GetKey() == EKeys::Gamepad_RightStick_Down ? 1.0f : -1.0f));
}

void SPanZoomPanel::HandleClick(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent)
{
	
}

FReply SPanZoomPanel::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) 
{
	if (IsPanAction_OnMouseButtonDown(MyGeometry, MouseEvent))
	{
		bIsPanning = true;
		HandlePan(MyGeometry, MouseEvent);
		return FReply::Handled().CaptureMouse(SharedThis(this)).UseHighPrecisionMouseMovement(SharedThis(this));
	}
	
	if (IsZoomAction_OnMouseButtonDown(MyGeometry, MouseEvent))
	{
		bIsZooming = true;
		HandleZoom(MyGeometry, MouseEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnMouseButtonDown(MyGeometry, MouseEvent))
	{
		bIsClicking = true;
		HandleClick(MyGeometry, MouseEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) 
{
	if (IsPanAction_OnMouseButtonUp(MyGeometry, MouseEvent))
	{
		bIsPanning = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	if (IsZoomAction_OnMouseButtonUp(MyGeometry, MouseEvent))
	{
		bIsZooming = false;
		return FReply::Handled();
	}

	if (IsClickAction_OnMouseButtonUp(MyGeometry, MouseEvent))
	{
		bIsClicking = false;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) 
{
	if (IsPanAction_OnMouseMove(MyGeometry, MouseEvent))
	{
		HandlePan(MyGeometry, MouseEvent);
		return FReply::Handled();
	}

	if (IsZoomAction_OnMouseMove(MyGeometry, MouseEvent))
	{
		HandleZoom(MyGeometry, MouseEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnMouseMove(MyGeometry, MouseEvent))
	{
		HandleClick(MyGeometry, MouseEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) 
{
	if (IsPanAction_OnMouseWheel(MyGeometry, MouseEvent))
	{
		bIsPanning = true;
		HandlePan(MyGeometry, MouseEvent);
		return FReply::Handled().CaptureMouse(SharedThis(this)).UseHighPrecisionMouseMovement(SharedThis(this));
	}

	if (IsZoomAction_OnMouseWheel(MyGeometry, MouseEvent))
	{
		bIsZooming = true;
		HandleZoom(MyGeometry, MouseEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnMouseWheel(MyGeometry, MouseEvent))
	{
		bIsClicking = true;
		HandleClick(MyGeometry, MouseEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& GestureEvent) 
{
	if (IsPanAction_OnTouchGesture(MyGeometry, GestureEvent))
	{
		bIsPanning = true;
		HandlePan(MyGeometry, GestureEvent);
		return FReply::Handled();
	}

	if (IsZoomAction_OnTouchGesture(MyGeometry, GestureEvent))
	{
		bIsZooming = true;
		HandleZoom(MyGeometry, GestureEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnTouchGesture(MyGeometry, GestureEvent))
	{
		bIsClicking = true;
		HandleClick(MyGeometry, GestureEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) 
{
	if (IsPanAction_OnTouchStarted(MyGeometry, InTouchEvent))
	{
		bIsPanning = true;
		HandlePan(MyGeometry, InTouchEvent);
		return FReply::Handled();
	}

	if (IsZoomAction_OnTouchStarted(MyGeometry, InTouchEvent))
	{
		bIsZooming = true;
		HandleZoom(MyGeometry, InTouchEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnTouchStarted(MyGeometry, InTouchEvent))
	{
		bIsClicking = true;
		HandleClick(MyGeometry, InTouchEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) 
{
	if (IsPanAction_OnTouchEnded(MyGeometry, InTouchEvent))
	{
		bIsPanning = false;
		return FReply::Handled();
	}

	if (IsZoomAction_OnTouchEnded(MyGeometry, InTouchEvent))
	{
		bIsZooming = false;
		return FReply::Handled();
	}

	if (IsClickAction_OnTouchEnded(MyGeometry, InTouchEvent))
	{
		bIsClicking = false;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) 
{
	if (IsPanAction_OnTouchMoved(MyGeometry, InTouchEvent))
	{
		bIsPanning = true;
		HandlePan(MyGeometry, InTouchEvent);
		return FReply::Handled();
	}

	if (IsZoomAction_OnTouchMoved(MyGeometry, InTouchEvent))
	{
		bIsZooming = true;
		HandleZoom(MyGeometry, InTouchEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnTouchMoved(MyGeometry, InTouchEvent))
	{
		bIsClicking = true;
		HandleClick(MyGeometry, InTouchEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) 
{
	if (IsPanAction_OnKeyDown(MyGeometry, KeyEvent))
	{
		bIsPanning = true;
		HandlePan(MyGeometry, KeyEvent);
		return FReply::Handled();
	}

	if (IsZoomAction_OnKeyDown(MyGeometry, KeyEvent))
	{
		bIsZooming = true;
		HandleZoom(MyGeometry, KeyEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnKeyDown(MyGeometry, KeyEvent))
	{
		bIsClicking = true;
		HandleClick(MyGeometry, KeyEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) 
{
	if (IsPanAction_OnKeyUp(MyGeometry, KeyEvent))
	{
		bIsPanning = false;
		return FReply::Handled();
	}

	if (IsZoomAction_OnKeyUp(MyGeometry, KeyEvent))
	{
		bIsZooming = false;
		return FReply::Handled();
	}

	if (IsClickAction_OnKeyUp(MyGeometry, KeyEvent))
	{
		bIsClicking = false;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPanZoomPanel::OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) 
{
	if (IsPanAction_OnAnalogValueChanged(MyGeometry, InAnalogInputEvent))
	{
		bIsPanning = true;
		HandlePan(MyGeometry, InAnalogInputEvent);
		return FReply::Handled().SetUserFocus(SharedThis(this));
	}

	if (IsZoomAction_OnAnalogValueChanged(MyGeometry, InAnalogInputEvent))
	{
		bIsZooming = true;
		HandleZoom(MyGeometry, InAnalogInputEvent);
		return FReply::Handled();
	}

	if (IsClickAction_OnAnalogValueChanged(MyGeometry, InAnalogInputEvent))
	{
		bIsClicking = true;
		HandleClick(MyGeometry, InAnalogInputEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FCursorReply SPanZoomPanel::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	return bIsPanning ? FCursorReply::Cursor(EMouseCursor::GrabHandClosed) : FCursorReply::Cursor(EMouseCursor::Default);
}

bool SPanZoomPanel::SupportsKeyboardFocus() const
{
	return true;
}

float SPanZoomPanel::GetZoom() const
{
	return ZoomDriver.CurrentZoom;
}

FVector2D SPanZoomPanel::GetViewOffset() const
{
	return PanDriver.CurrentViewOffset;
}

FVector2D SPanZoomPanel::GetViewCenter() const
{
	return GetViewRect().GetCenter();
}

FVector2D SPanZoomPanel::GetWidgetCenter() const
{
	return GetWidgetRect().GetCenter();
}

FVector2D SPanZoomPanel::ToWidgetPosition(const FVector2D& ViewPosition) const
{
	return (ViewPosition - GetViewOffset()) * GetZoom();
}

FVector2D SPanZoomPanel::ToViewPosition(const FVector2D& WidgetPosition) const
{
	return WidgetPosition / GetZoom() + GetViewOffset();
}

float SPanZoomPanel::AngleToViewCenter(const FVector2D& Position, bool bIsInViewSpace) const
{
	return FMath::Acos(GetViewCenter() | (bIsInViewSpace ? Position : ToViewPosition(Position)));
}

void SPanZoomPanel::Pan(const FVector2D& PanAmountAndDirection)
{
	PanDriver.TargetViewOffset += (PanAmountAndDirection / GetZoom());
}

void SPanZoomPanel::Zoom(float ZoomAmount)
{
	ZoomDriver.AddZoom(ZoomAmount);
}

void SPanZoomPanel::SnapToZoom(float ZoomValue)
{
	ZoomDriver.SetZoom(ZoomValue);
	ZoomDriver.UpdateZoom(5000.0f);
}

void SPanZoomPanel::PanTo(const FVector2D& DesiredViewPosition)
{
	const FVector2D ViewHalfSize = 0.5f * LastTickGeometry.Size / GetZoom();
	FVector2D CurrentPosition = GetViewOffset() + ViewHalfSize;

	PanDriver.TargetViewOffset = DesiredViewPosition - ViewHalfSize;
}

void SPanZoomPanel::SnapToViewPosition(const FVector2D& ViewPosition)
{
	PanTo(ViewPosition);
	PanDriver.UpdatePan(5000.0f);
}

FSlateRect SPanZoomPanel::GetViewRect() const
{
	const FVector2D Size = LastTickGeometry.GetLocalSize() / GetZoom();
	FSlateRect Rect(FVector2D::ZeroVector, Size);
	return Rect.OffsetBy(GetViewOffset());
}

FSlateRect SPanZoomPanel::GetWidgetRect() const
{
	return LastTickGeometry.GetClippingRect();
}

void SPanZoomPanel::SetZoomSpeed(float NewSpeed)
{
	ZoomDriver.ZoomSpeed = NewSpeed;
}

void SPanZoomPanel::SetPanSpeed(float NewSpeed)
{
	PanDriver.PanSpeed = NewSpeed;
}

void SPanZoomPanel::SetZoomRanges(float Min, float Max, const TArray<float>& Ranges)
{
	ZoomDriver.Min = Min;
	ZoomDriver.Max = Max;
	ZoomDriver.ZoomValues = Ranges;
}
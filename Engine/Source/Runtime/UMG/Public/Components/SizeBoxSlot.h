// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateWrapperTypes.h"

#include "SizeBoxSlot.generated.h"

/** The Slot for the USizeBoxSlot, contains the widget displayed in a button's single slot */
UCLASS()
class UMG_API USizeBoxSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()

public:
	/** The padding area between the slot and the content it contains. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Layout (SizeBox Slot)")
	FMargin Padding;

	/** The alignment of the object horizontally. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Layout (SizeBox Slot)")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

	/** The alignment of the object vertically. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Layout (SizeBox Slot)")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment;

public:

	UFUNCTION(BlueprintCallable, Category="Layout (SizeBox Slot)")
	void SetPadding(FMargin InPadding);

	UFUNCTION(BlueprintCallable, Category="Layout (SizeBox Slot)")
	void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	UFUNCTION(BlueprintCallable, Category="Layout (SizeBox Slot)")
	void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

public:

	// UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// End of UPanelSlot interface

	/** Builds the underlying slot for the slate SizeBox. */
	void BuildSlot(TSharedRef<SBox> InSizeBox);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:

	/** A pointer to the button to allow us to adjust the size, padding...etc at runtime. */
	TSharedPtr<SBox> SizeBox;
};

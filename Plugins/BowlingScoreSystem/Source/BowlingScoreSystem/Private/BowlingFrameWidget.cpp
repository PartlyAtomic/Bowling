﻿// Partly Atomic LLC 2025


#include "BowlingFrameWidget.h"

#include "BowlingScoreComponent.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

UBowlingFrameWidget::UBowlingFrameWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer), FrameNumber(0)
{
	SetIsFocusable(true);
}

void UBowlingFrameWidget::SetFrame_Implementation(int32 InFrameNumber)
{
	FrameNumber = InFrameNumber;

	// No reason to keep the third shot textbox around unless this is the final frame
	if (not IsFinalFrame())
	{
		Shot3TextBox->RemoveFromParent();
		Shot3TextBox = nullptr;
	}
}

void UBowlingFrameWidget::Reset()
{
	// Reset textboxes to disabled input and clear contents
	for (auto&& TextBox : {Shot1TextBox, Shot2TextBox, Shot3TextBox})
	{
		if (!IsValid(TextBox)) { continue; }
		TextBox->SetText(FText::GetEmpty());
		TextBox->SetIsEnabled(false);
		TextBox->SetIsReadOnly(true);
	}

	Shot1TextBox->OnTextChanged.AddUniqueDynamic(this, &UBowlingFrameWidget::ValidateShot1Entry);
	Shot2TextBox->OnTextChanged.AddUniqueDynamic(this, &UBowlingFrameWidget::ValidateShot2Entry);
	if (Shot3TextBox)
	{
		Shot3TextBox->OnTextChanged.AddUniqueDynamic(this, &UBowlingFrameWidget::ValidateShot3Entry);
	}

	ScoreText->SetText(FText::GetEmpty());
}

void UBowlingFrameWidget::SetCurrentGameState(int32 CurrentFrame, int32 CurrentShot)
{
	TArray<UEditableTextBox*> ShotBoxes = {Shot1TextBox, Shot2TextBox, Shot3TextBox};
	auto CurrentShotIndex = CurrentShot - 1;
	if (CurrentFrame == FrameNumber)
	{
		if (not ensure(ShotBoxes.IsValidIndex(CurrentShotIndex))) { return; }
		auto* ShotBox = ShotBoxes[CurrentShotIndex];
		if (not ensure(IsValid(ShotBox))) { return; }

		// The textboxes will be disabled after submitting a score
		ShotBox->SetIsReadOnly(false);
		ShotBox->SetIsEnabled(true);
		SetDesiredFocusWidget(ShotBox);
		SetFocus();
	}
}

void UBowlingFrameWidget::UpdateScore()
{
	// Only set score text if at least one shot has been entered for the current frame
	if (not Shot1TextBox->GetText().IsEmpty())
	{
		auto* BowlingScoreComponent = GetBowlingScoreComponent();
		if (not ensure(IsValid(BowlingScoreComponent))) { return; }

		auto Score = BowlingScoreComponent->GetScore(FrameNumber);

		ScoreText->SetText(FText::Format(INVTEXT("{0}"), {Score}));
	}
}

bool UBowlingFrameWidget::IsFinalFrame() const
{
	return FrameNumber == 10;
}

UBowlingScoreComponent* UBowlingFrameWidget::GetBowlingScoreComponent() const
{
	// BowlingScoreComponent lives on PlayerState
	auto* PlayerState = GetOwningPlayerState();
	if (not ensure(IsValid(PlayerState))) { return nullptr; }

	return PlayerState->GetComponentByClass<UBowlingScoreComponent>();
}

void UBowlingFrameWidget::ValidateShot1Entry(const FText& Text)
{
	ValidateTextEntry(Text, 1);
}

void UBowlingFrameWidget::ValidateShot2Entry(const FText& Text)
{
	ValidateTextEntry(Text, 2);
}

void UBowlingFrameWidget::ValidateShot3Entry(const FText& Text)
{
	ValidateTextEntry(Text, 3);
}

void UBowlingFrameWidget::ValidateTextEntry(const FText& Text, int32 Shot)
{
	// Empty is OK
	if (Text.IsEmpty()) { return; }

	TArray<UEditableTextBox*> TextBoxes = {Shot1TextBox, Shot2TextBox, Shot3TextBox};

	if (not ensure(TextBoxes.IsValidIndex(Shot - 1))) { return; }
	auto* TextBox = TextBoxes[Shot - 1];

	if (not ensureMsgf(IsValid(TextBox), TEXT("Could not find modified textbox")))
	{
		return;
	}

	// Only accept the first character
	auto StringText = Text.ToString();
	if (StringText.Len() > 1)
	{
		StringText = StringText.Left(1);
		TextBox->SetText(FText::FromString(StringText));
	}

	auto* BowlingScoreComponent = GetBowlingScoreComponent();
	if (not ensure(IsValid(BowlingScoreComponent)))
	{
		TextBox->SetText(FText::GetEmpty());
		return;
	}

	// Custom IsNumeric function since FString::IsNumeric will accept ".", "+", and "-"
	auto IsNumeric = [](const FString& String) -> bool
	{
		if (String.IsEmpty()) { return false; }
		
		for (auto&& C : String)
		{
			if (C < '0' or C > '9') { return false; }
		}

		return true;
	};
	
	auto Success = false;

	if (StringText == "X" or StringText == "x" or StringText == "/")
	{
		// Convert spare to numeric representations and validate score
		if (StringText == "/")
		{
			// Convert spare into a number.
			if (IsFinalFrame())
			{
				if (Shot == 2 or Shot == 3)
				{
					// Can only hit a spare if the previous shot wasn't a strike
					if (not BowlingScoreComponent->IsStrike(FrameNumber, Shot - 1))
					{
						auto SpareScore = 10 - BowlingScoreComponent->GetShotScore(FrameNumber, Shot - 1);
						Success = BowlingScoreComponent->SetScore(SpareScore);
					}
				}
			}
			else if (Shot == 2)
			{
				// 10 - Shot 1
				auto SpareScore = 10 - BowlingScoreComponent->GetShotScore(FrameNumber, 1);
				Success = BowlingScoreComponent->SetScore(SpareScore);
			}
		}
		else
		{
			// Strike case
			if (Shot == 1)
			{
				// Strikes are always allowed on Shot 11
				Success = BowlingScoreComponent->SetScore(10);
			}
			else if (IsFinalFrame())
			{
				// Or on Frame 10, strike is allowed ....
				// For Shot 3 if Shot 2 was a spare
				// or Shot 3 if Shot 1 and 2 were strikes
				// or Shot 2 if Shot 1 was a strike
				if ((Shot == 3 and BowlingScoreComponent->IsSpare(10, 2))
					or (Shot == 3 and BowlingScoreComponent->IsStrike(10, 1) and BowlingScoreComponent->IsStrike(10, 2))
					or (Shot == 2 and BowlingScoreComponent->IsStrike(10, 1)))
				{
					Success = BowlingScoreComponent->SetScore(10);
				}
			}
		}
	}
	else if (IsNumeric(StringText))
	{
		auto Score = FCString::Atoi(*StringText);
		Success = BowlingScoreComponent->SetScore(Score);

		// Convert number to / or X notation
		if (Success)
		{
			if (BowlingScoreComponent->IsSpare(FrameNumber, Shot)) { TextBox->SetText(INVTEXT("/")); }
			else if (BowlingScoreComponent->IsStrike(FrameNumber, Shot)) { TextBox->SetText(INVTEXT("X")); }
		}
	}

	if (Success)
	{
		// Disable the box, focus will be transferred to the next box with SetCurrentGameState
		TextBox->SetIsReadOnly(true);
		TextBox->SetIsEnabled(false);
	}
	else
	{
		TextBox->SetText(FText::GetEmpty());
	}
}

void UBowlingFrameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Reset();
}

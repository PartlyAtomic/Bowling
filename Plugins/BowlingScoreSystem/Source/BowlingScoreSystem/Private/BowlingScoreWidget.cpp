// Partly Atomic LLC 2025

#include "BowlingScoreWidget.h"

#include "BowlingFrameWidget.h"
#include "BowlingScoreComponent.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/PlayerState.h"

UBowlingScoreWidget::UBowlingScoreWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsFocusable(true);
	BowlingFrameWidgetClass = UBowlingFrameWidget::StaticClass();
}

UBowlingScoreComponent* UBowlingScoreWidget::GetBowlingScoreComponent() const
{
	// The BowlingScoreComponent lives on PlayerState
	auto* PlayerState = GetOwningPlayerState();
	if (not ensure(PlayerState)) { return nullptr; }

	return PlayerState->GetComponentByClass<UBowlingScoreComponent>();
}

void UBowlingScoreWidget::Reset()
{
	// Reset all score frames
	for (auto&& Child : FrameBox->GetAllChildren())
	{
		auto* FrameWidget = Cast<UBowlingFrameWidget>(Child);
		if (not ensure(IsValid(FrameWidget))) { continue; }

		FrameWidget->Reset();
	}

	// Restart the game
	auto* BowlingScoreComponent = GetBowlingScoreComponent();
	if (not ensure(IsValid(BowlingScoreComponent))) { return; }
	BowlingScoreComponent->Reset();
}

void UBowlingScoreWidget::GameAdvanced(UBowlingScoreComponent* BowlingScoreComponent, int32 Frame, int32 Shot)
{
	// Scores will at most change the previous two scores. Since this could be the start of the "next" frame, go back three.
	const auto UpdateFrom = FMath::Max(0, Frame - 1 - 3);
	const auto UpdateTo = Frame;

	for (auto ChildNum = 0; ChildNum < FrameBox->GetChildrenCount(); ChildNum++)
	{
		auto* FrameWidget = Cast<UBowlingFrameWidget>(FrameBox->GetChildAt(ChildNum));
		if (not ensure(IsValid(FrameWidget))) { continue; }

		FrameWidget->SetCurrentGameState(Frame, Shot);

		// Update the score if the frames are in the range that the last shot could have modified their score
		if (ChildNum >= UpdateFrom and ChildNum < UpdateTo)
		{
			FrameWidget->UpdateScore();
		}
	}
}

void UBowlingScoreWidget::GameOver(UBowlingScoreComponent* BowlingScoreComponent)
{
	// Score needs updated for frame 10 and possibly frame 9 on game over
	auto* FrameNineWidget = Cast<UBowlingFrameWidget>(FrameBox->GetChildAt(8));
	FrameNineWidget->UpdateScore();
	auto* FrameTenWidget = Cast<UBowlingFrameWidget>(FrameBox->GetChildAt(9));
	FrameTenWidget->UpdateScore();

	// Focus on the reset button
	SetDesiredFocusWidget(ResetButton);
	SetFocus();
}

void UBowlingScoreWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (not ensure(FrameBox)) { return; }
	if (not ensure(BowlingFrameWidgetClass)) { return; }

	// Since I expect frame numbers to line up with the horizontal box's children,
	// clear anything that might have been put in via designer
	FrameBox->ClearChildren();

	// Create all the frame widgets
	for (auto i = 0; i < 10; i++)
	{
		auto* FrameWidget = CreateWidget<UBowlingFrameWidget>(this, BowlingFrameWidgetClass,
		                                                      FName("Frame" + FString::FromInt(i + 1)));
		FrameWidget->SetFrame(i + 1);
		FrameBox->AddChild(FrameWidget);
	}

	// Bind to reset button
	if (not ensure(ResetButton)) { return; }
	ResetButton->OnClicked.AddUniqueDynamic(this, &UBowlingScoreWidget::Reset);
}

void UBowlingScoreWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto* BowlingScoreComponent = GetBowlingScoreComponent();
	if (not ensure(BowlingScoreComponent)) { return; }

	// Listen to relevant game events and "start" the game
	BowlingScoreComponent->OnGameAdvanced.AddUniqueDynamic(this, &UBowlingScoreWidget::GameAdvanced);
	BowlingScoreComponent->OnGameOver.AddUniqueDynamic(this, &UBowlingScoreWidget::GameOver);
	BowlingScoreComponent->Reset();

	// Set focus to the first frame
	if (not ensure(FrameBox)) { return; }
	SetDesiredFocusWidget(FrameBox->GetChildAt(0));
	SetFocus();
}

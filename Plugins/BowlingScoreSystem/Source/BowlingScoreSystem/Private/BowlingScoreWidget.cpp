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
	auto* PlayerController = GetOwningPlayer();
	if (!ensure(PlayerController)) { return nullptr; }

	auto* PlayerState = PlayerController->GetPlayerState<APlayerState>();
	if (!ensure(PlayerState)) { return nullptr; }

	return PlayerState->GetComponentByClass<UBowlingScoreComponent>();
}

void UBowlingScoreWidget::Reset()
{
	for (auto&& Child : FrameBox->GetAllChildren())
	{
		auto* FrameWidget = Cast<UBowlingFrameWidget>(Child);
		if (!ensure(FrameWidget)) { continue; }

		FrameWidget->Reset();
	}
	auto* BowlingScoreComponent = GetBowlingScoreComponent();
	
	if (!ensure(BowlingScoreComponent)) { return; }
	BowlingScoreComponent->Reset();
}

void UBowlingScoreWidget::GameAdvanced(UBowlingScoreComponent* BowlingScoreComponent, int32 Frame, int32 Shot)
{
	// Scores will at most change the previous two scores. Since this could be the start of the "next" frame, go back three.
	auto UpdateFrom = FMath::Max(0, Frame-1-3);
	auto UpdateTo = Frame;
	for (auto ChildNum = 0; ChildNum < FrameBox->GetChildrenCount(); ChildNum++)
	{
		auto* FrameWidget = Cast<UBowlingFrameWidget>(FrameBox->GetChildAt(ChildNum));
		if (!ensure(FrameWidget)) { continue; }

		FrameWidget->SetCurrentGameState(Frame, Shot);
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
	SetDesiredFocusWidget(ResetButton);
	// ResetButton->SetFocus();
	SetFocus();
}

void UBowlingScoreWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!ensure(FrameBox)) { return; }
	if (!ensure(BowlingFrameWidgetClass)) { return; }

	for (auto i = 0; i < 10; i++)
	{
		auto* FrameWidget = CreateWidget<UBowlingFrameWidget>(this, BowlingFrameWidgetClass,
		                                                      FName("Frame" + FString::FromInt(i + 1)));
		FrameWidget->SetFrame(i + 1);
		FrameBox->AddChild(FrameWidget);
	}

	if (!ensure(ResetButton)) { return; }
	ResetButton->OnClicked.AddUniqueDynamic(this, &UBowlingScoreWidget::Reset);
}

void UBowlingScoreWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto* BowlingScoreComponent = GetBowlingScoreComponent();
	if (!ensure(BowlingScoreComponent)) { return; }

	BowlingScoreComponent->OnGameAdvanced.AddUniqueDynamic(this, &UBowlingScoreWidget::GameAdvanced);
	BowlingScoreComponent->OnGameOver.AddUniqueDynamic(this, &UBowlingScoreWidget::GameOver);
	BowlingScoreComponent->Reset();
	SetDesiredFocusWidget(FrameBox->GetChildAt(0));
	SetFocus();
}

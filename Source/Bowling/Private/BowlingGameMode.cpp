// Partly Atomic LLC 2025

#include "Bowling/Public/BowlingGameMode.h"

#include "BowlingPlayerState.h"
#include "Blueprint/UserWidget.h"

ABowlingGameMode::ABowlingGameMode()
{
	PlayerStateClass = ABowlingPlayerState::StaticClass();
}

void ABowlingGameMode::BeginPlay()
{
	Super::BeginPlay();

	// For the sake of simplicity, only displaying the first player's score
	auto* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController and ensure(BowlingScoreWidgetClass))
	{
		auto* BowlingScoreWidget = CreateWidget<UUserWidget>(PlayerController, BowlingScoreWidgetClass);
		if (ensure(BowlingScoreWidget))
		{
			BowlingScoreWidget->AddToViewport();

			// Set focus 
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(BowlingScoreWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			PlayerController->bShowMouseCursor = true;
		
		}
	}
	
}

// Partly Atomic LLC 2025

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BowlingScoreWidget.generated.h"

class UBowlingScoreComponent;
class UButton;
class UHorizontalBox;
class UBowlingFrameWidget;

/**
 * Bowling score widget to represent scoring for one player.
 * Score can only be entered for the current shot
 * Scorecard can be reset
 */
UCLASS()
class BOWLINGSCORESYSTEM_API UBowlingScoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBowlingScoreWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UBowlingScoreComponent* GetBowlingScoreComponent() const;

	// Reset the game back to the first shot and clear the scorecard
	UFUNCTION()
	void Reset();

	// Listen for the game to be advanced
	UFUNCTION()
	void GameAdvanced(UBowlingScoreComponent* BowlingScoreComponent, int32 Frame, int32 Shot);

	// Listen for the game ending
	UFUNCTION()
	void GameOver(UBowlingScoreComponent* BowlingScoreComponent);

	// This HorizontalBox holds all the frame widgets
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> FrameBox;

	// Button for resetting the score
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UButton> ResetButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Bowling)
	TSubclassOf<UBowlingFrameWidget> BowlingFrameWidgetClass;

protected:
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;
};

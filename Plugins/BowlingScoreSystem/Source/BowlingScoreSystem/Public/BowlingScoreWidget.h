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
 * 
 */
UCLASS()
class BOWLINGSCORESYSTEM_API UBowlingScoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBowlingScoreWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UBowlingScoreComponent* GetBowlingScoreComponent() const;

	UFUNCTION()
	void Reset();
	
	UFUNCTION()
	void GameAdvanced(UBowlingScoreComponent* BowlingScoreComponent, int32 Frame, int32 Shot);

	UFUNCTION()
	void GameOver(UBowlingScoreComponent* BowlingScoreComponent);
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> FrameBox;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UButton> ResetButton;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Bowling)
	TSubclassOf<UBowlingFrameWidget> BowlingFrameWidgetClass;
	
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;
};

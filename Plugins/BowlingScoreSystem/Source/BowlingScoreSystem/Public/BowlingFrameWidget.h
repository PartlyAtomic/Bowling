// Partly Atomic LLC 2025

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BowlingFrameWidget.generated.h"

class UTextBlock;
class UBowlingScoreComponent;
class UEditableTextBox;
/**
 * Widget for a single frame of a bowling game.
 */
UCLASS()
class BOWLINGSCORESYSTEM_API UBowlingFrameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBowlingFrameWidget(const FObjectInitializer& ObjectInitializer);

	// Set the frame the widget represents, this allows for customization on the final frame
	UFUNCTION(BlueprintNativeEvent, Category=Bowling)
	void SetFrame(int32 InFrameNumber);

	UFUNCTION(BlueprintCallable, Category=Bowling)
	void Reset();
	
	UFUNCTION(BlueprintCallable, Category=Bowling)
	void SetCurrentGameState(int32 CurrentFrame, int32 CurrentShot);

	UFUNCTION(BlueprintCallable, Category=Bowling)
	void UpdateScore();
	
	UFUNCTION(BlueprintCallable, Category=Bowling)
	bool IsFinalFrame() const;

	int32 GetFrameNumber() const { return FrameNumber; }
protected:
	UBowlingScoreComponent* GetBowlingScoreComponent() const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Bowling)
	int32 FrameNumber;
	
	UPROPERTY(BlueprintReadWrite, Category=Bowling, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Shot1TextBox;

	UPROPERTY(BlueprintReadWrite, Category=Bowling, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Shot2TextBox;

	UPROPERTY(BlueprintReadWrite, Category=Bowling, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> Shot3TextBox;

	UPROPERTY(BlueprintReadWrite, Category=Bowling, meta=(BindWidget))
	TObjectPtr<UTextBlock> ScoreText;
	
	UFUNCTION()
	void ValidateShot1Entry(const FText& Text);

	UFUNCTION()
	void ValidateShot2Entry(const FText& Text);
	
	UFUNCTION()
	void ValidateShot3Entry(const FText& Text);
	
	void ValidateTextEntry(const FText& Text, int32 Shot);
	
	virtual void NativeConstruct() override;
	
};

// Partly Atomic LLC 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BowlingGameMode.generated.h"

/**
 * Bare bones GameMode for a bowling game
 */
UCLASS()
class BOWLING_API ABowlingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABowlingGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Classes)
	TSubclassOf<UUserWidget> BowlingScoreWidgetClass;

protected:
	virtual void BeginPlay() override;
};

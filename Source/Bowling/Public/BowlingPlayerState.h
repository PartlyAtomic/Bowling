// Partly Atomic LLC 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BowlingPlayerState.generated.h"

class UBowlingScoreComponent;
/**
 * Bare bones player state for bowling
 */
UCLASS()
class BOWLING_API ABowlingPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	ABowlingPlayerState();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Bowling)
	TObjectPtr<UBowlingScoreComponent> BowlingScoreComponent;

};

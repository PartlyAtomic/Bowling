// Partly Atomic LLC 2025

#include "Bowling/Public/BowlingPlayerState.h"

#include "BowlingScoreComponent.h"

ABowlingPlayerState::ABowlingPlayerState()
{
	BowlingScoreComponent = CreateDefaultSubobject<UBowlingScoreComponent>(TEXT("Bowling Score Component"));
}

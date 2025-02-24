// Partly Atomic LLC 2025

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BowlingScoreComponent.generated.h"

USTRUCT()
struct FBowlingFrameScore
{
	GENERATED_BODY()

	FBowlingFrameScore();

	// Utility constructor primarily for testing
	FBowlingFrameScore(int32 Shot1, int32 Shot2);

	// Utility constructor primarily for testing
	FBowlingFrameScore(int32 Shot1, int32 Shot2, int32 Shot3);

	TArray<int32, TInlineAllocator<3>> Shots;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBowlingScoreChangedSignature, UBowlingScoreComponent*, BowlingScoreComponent, int32, Frame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBowlingResetSignature, UBowlingScoreComponent*, BowlingScoreComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBowlingGameAdvancedSignature, UBowlingScoreComponent*, BowlingScoreComponent, int32, Frame, int32, Shot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBowlingGameOverSignature, UBowlingScoreComponent*, BowlingScoreComponent);

/*
 * Component for PlayerState (or anywhere really) to keep track of bowling score.
 */
UCLASS()
class BOWLINGSCORESYSTEM_API UBowlingScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBowlingScoreComponent();

	// Reset the bowling game, starting over on Frame 1 Shot 1
	UFUNCTION(BlueprintCallable, Category=Bowling)
	void Reset();

	// Get the current frame. Returns -1 during game over.
	UFUNCTION(BlueprintCallable, Category=Bowling)
	int32 GetCurrentFrameNum() const;

	// Get the current shot. Returns -1 during game over.
	UFUNCTION(BlueprintCallable, Category=Bowling)
	int32 GetCurrentShotNum() const;

	// Get the total score as of a specific frame
	UFUNCTION(BlueprintCallable, Category=Bowling)
	int32 GetScore(int32 Frame) const;

	// Get the specified shot's score
	UFUNCTION(BlueprintCallable, Category=Bowling)
	int32 GetShotScore(int32 Frame, int32 Shot) const;

	// Get the specified frame's score
	UFUNCTION(BlueprintCallable, Category=Bowling)
	int32 GetFrameScore(int32 Frame) const;

	// Check if a shot score is valid.
	// Note: This will assume future shots in a frame are zero if a previous shot is entered
	// For example, if shot 1 on Frame 10 is entered, the game state will be assumed to be at Frame 10 Shot 1
	UFUNCTION(BlueprintCallable, Category=Bowling)
	bool IsValidShotScore(int32 Score, int32 Frame, int32 Shot) const;

	// Attempt to set the score for the current frame and shot
	// Return indicates whether the score was accepted
	// Broadcasts OnGameAdvanced when score is recorded and game moves to the next shot 
	// Broadcasts OnReset after the final score is recorded
	UFUNCTION(BlueprintCallable, Category=Bowling)
	bool SetScore(int32 Score);

	UFUNCTION(BlueprintCallable, Category=Bowling)
	bool IsSpare(int32 Frame, int32 Shot) const;

	UFUNCTION(BlueprintCallable, Category=Bowling)
	bool IsStrike(int32 Frame, int32 Shot) const;

	UFUNCTION(BlueprintCallable, Category=Bowling)
	bool IsGameOver() const;

	// Broadcast when the game is reset
	UPROPERTY(BlueprintAssignable)
	FOnBowlingResetSignature OnReset;

	// Broadcast when the game advances to the next shot
	// Note: does not broadcast after the last shot
	UPROPERTY(BlueprintAssignable)
	FOnBowlingGameAdvancedSignature OnGameAdvanced;

	// Broadcast when the game is completed
	UPROPERTY(BlueprintAssignable)
	FOnBowlingGameOverSignature OnGameOver;

protected:
	// Set the score for a given Frame and Shot
	// Note: Due to time constraints this will only work for the current frame and shot, so it's not exposed
	bool SetScore(int32 Score, int32 Frame, int32 Shot);

	// Get the next shot from a given Frame and Shot that will be used for spare and strike scoring.
	TPair<int32, int32> GetNextScoredShot(int32 Frame, int32 Shot) const;

	// Allow the testing class to manipulate internals for test setup
	friend struct BowlingScoreTests;

	TArray<FBowlingFrameScore> FrameScores;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentFrameIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentShotIndex;

public:
	virtual void InitializeComponent() override;
};

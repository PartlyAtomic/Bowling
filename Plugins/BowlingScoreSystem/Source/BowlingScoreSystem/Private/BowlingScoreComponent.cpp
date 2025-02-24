// Partly Atomic LLC 2025
#include "BowlingScoreComponent.h"

FBowlingFrameScore::FBowlingFrameScore()
{
	Shots.SetNum(3);
}

FBowlingFrameScore::FBowlingFrameScore(int32 Shot1, int32 Shot2) : FBowlingFrameScore()
{
	Shots = {Shot1, Shot2};
}

FBowlingFrameScore::FBowlingFrameScore(int32 Shot1, int32 Shot2, int32 Shot3) : FBowlingFrameScore()
{
	Shots = {Shot1, Shot2, Shot3};
}

UBowlingScoreComponent::UBowlingScoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Reset();
}

void UBowlingScoreComponent::Reset()
{
	// Reset shot and frame
	CurrentFrameIndex = 0;
	CurrentShotIndex = 0;

	// Reset frames
	FrameScores.Reset(10);
	FrameScores.SetNum(10);
	for (auto FrameIdx = 0; FrameIdx < 10; FrameIdx++)
	{
		FrameScores[FrameIdx].Shots.SetNum(FrameIdx == 9 ? 3 : 2);
	}

	// Broadcast reset and advance to first shot
	OnReset.Broadcast(this);
	OnGameAdvanced.Broadcast(this, GetCurrentFrameNum(), GetCurrentShotNum());
}

int32 UBowlingScoreComponent::GetCurrentFrameNum() const
{
	if (IsGameOver()) { return -1; }
	return CurrentFrameIndex + 1;
}

int32 UBowlingScoreComponent::GetCurrentShotNum() const
{
	if (IsGameOver()) { return -1; }
	return CurrentShotIndex + 1;
}

int32 UBowlingScoreComponent::GetScore(int32 Frame) const
{
	auto Score = 0;

	for (auto CurrentFrame = 1; CurrentFrame <= Frame; CurrentFrame++)
	{
		Score += GetFrameScore(CurrentFrame);
	}

	return Score;
}

int32 UBowlingScoreComponent::GetShotScore(int32 Frame, int32 Shot) const
{
	auto FrameIdx = Frame - 1;
	auto ShotIdx = Shot - 1;

	if (not FrameScores.IsValidIndex(FrameIdx)) { return 0; }
	auto& CurrentFrame = FrameScores[FrameIdx];
	if (not CurrentFrame.Shots.IsValidIndex(ShotIdx)) { return 0; }

	return CurrentFrame.Shots[ShotIdx];
}

int32 UBowlingScoreComponent::GetFrameScore(int32 Frame) const
{
	auto FrameIdx = Frame - 1;
	auto& CurrentFrame = FrameScores[FrameIdx];
	auto Score = 0;
	for (auto&& ShotScore : CurrentFrame.Shots)
	{
		Score += ShotScore;
	}

	// For once Frame 10 makes things easier
	if (Frame == 10)
	{
		return Score;
	}
	
	auto ShotsToSum = 0;
	if (IsStrike(Frame, 1))
	{
		// Add the score of the next two shots for a strike
		ShotsToSum = 2;
	}
	else if (IsSpare(Frame, 2))
	{
		// Add the score of the next one shot for a spare
		ShotsToSum = 1;
	}

	// Sum up extra scores as necessary
	if (ShotsToSum > 0)
	{
		TPair<int32, int32> NextShotPair = GetNextScoredShot(Frame, 2);
		for (auto i = 0; i < ShotsToSum; i++)
		{
			auto& NextFrame = NextShotPair.Get<0>();
			auto& NextShot = NextShotPair.Get<1>();
			Score += GetShotScore(NextFrame, NextShot);
			NextShotPair = GetNextScoredShot(NextFrame, NextShot);
		}
	}

	return Score;
}

bool UBowlingScoreComponent::IsValidShotScore(int32 Score, int32 Frame, int32 Shot) const
{
	auto FrameIdx = Frame - 1;
	auto ShotIdx = Shot - 1;
	
	// Number of pins must be between 0 and 10
	if (Score < 0 or Score > 10)
	{
		return false;
	}

	if (not FrameScores.IsValidIndex(FrameIdx)) { return false; }
	auto& CurrentFrame = FrameScores[FrameIdx];
	if (not CurrentFrame.Shots.IsValidIndex(ShotIdx)) { return false; }

	if (Frame <= 9)
	{
		// Frames 1-9 only have two shots
		if (Shot < 1 or Shot > 2) { return false; }

		// Don't allow shot values that would conflict with existing shots in the frame
		if (Shot == 2 and Score + CurrentFrame.Shots[0] > 10) { return false; }
	}
	else if (Frame == 10)
	{
		// Possible frame 10 states:
		// XXX, XXN, XNN, XN/
		// N/X, N/N
		// NN
		
		// Frame 10 only has max three shots
		if (Shot < 1 or Shot > 3) { return false; }

		// If Shot 1 was not a strike, then Shot 2 can only be up to a spare
		if (Shot == 2 and CurrentFrame.Shots[0] != 10 and (Score + CurrentFrame.Shots[0] > 10)) { return false; }

		// Thought: This might be covered by the previous rule..
		// Ten pins on shot 2 means Shot 1 must be a strike or 0
		// if (Shot == 2 and Score == 10 and not (CurrentFrame.Shots[0] == 10 or CurrentFrame.Shots[0] == 0)) { return false; }

		// Three shots are only allowed if the first shot of Frame 10 was a strike or the second shot was a spare
		if (Shot == 3 and not(CurrentFrame.Shots[0] == 10 or CurrentFrame.Shots[0] + CurrentFrame.Shots[1] == 10))
		{
			return false;
		}

		// If Shot 1 was a strike and Shot 2 wasn't (so the pins weren't reset for Shot 3), then Shot 3 can only be up to a spare
		if (Shot == 3 and CurrentFrame.Shots[0] == 10
			and CurrentFrame.Shots[1] != 10 and (Score + CurrentFrame.Shots[2]> 10)) { return false; }
	}

	return true;
}

bool UBowlingScoreComponent::IsValidShotScore(int32 Score) const
{
	return IsValidShotScore(Score, GetCurrentFrameNum(), GetCurrentShotNum());
}

bool UBowlingScoreComponent::SetScore(int32 Score)
{
	return SetScore(Score, GetCurrentFrameNum(), GetCurrentShotNum());
}

bool UBowlingScoreComponent::SetScore(int32 Score, int32 Frame, int32 Shot)
{
	auto FrameIdx = Frame - 1;
	auto ShotIdx = Shot - 1;

	if (not IsValidShotScore(Score, Frame, Shot))
	{
		return false;
	}

	// NOTE: I had thought about letting SetScore manipulate past scores, but that opens up a whole can of worms
	// about what the current frame/shot is. To keep things simple, IsValidShotScore is implemented assuming future
	// shots are zeroed out. This function will be implemented so you can only set score for the current frame/shot.
	if (FrameIdx != CurrentFrameIndex or ShotIdx != CurrentShotIndex)
	{
		return false;
	}

	// Record the score
	FrameScores[FrameIdx].Shots[ShotIdx] = Score;

	auto IsGameOver = false;

	// Advance shot and frame as necessary
	if (Frame < 10)
	{
		if (Shot == 1)
		{
			if (Score == 10)
			{
				// A strike will advance frame
				CurrentShotIndex = ShotIdx;
				CurrentFrameIndex = FrameIdx + 1;
			}
			else
			{
				// Everything else will advance to the next shot
				CurrentShotIndex = ShotIdx + 1;
				CurrentFrameIndex = FrameIdx;
			}
		}
		else if (Shot == 2)
		{
			// Shot 2 on frames other than 10 advances to the next frame
			CurrentShotIndex = 0;
			CurrentFrameIndex = FrameIdx + 1;
		}
	}
	else if (Frame == 10)
	{
		if (Shot == 1)
		{
			// Shot 1 always advances to the next shot
			CurrentShotIndex = ShotIdx + 1;
			CurrentFrameIndex = FrameIdx;
		}
		if (Shot == 2)
		{
			if (IsStrike(10, 1) or IsSpare(10, 2))
			{
				// Third shot is only allowed if there's a Strike on Shot 1 or Spare on Shot 2
				CurrentShotIndex = ShotIdx + 1;
				CurrentFrameIndex = FrameIdx;
			}
			else
			{
				// Otherwise it's game over
				CurrentFrameIndex = FrameIdx + 1;
				CurrentShotIndex = 0;
				IsGameOver = true;
			}
		}
		else if (Shot == 3)
		{
			// Third shot is always a game over
			CurrentFrameIndex = FrameIdx + 1;
			CurrentShotIndex = 0;
			IsGameOver = true;
		}
	}

	if (IsGameOver)
	{
		OnGameOver.Broadcast(this);
	}
	else
	{
		OnGameAdvanced.Broadcast(this, GetCurrentFrameNum(), GetCurrentShotNum());
	}
	return true;
}

bool UBowlingScoreComponent::IsSpare(int32 Frame, int32 Shot) const
{
	auto FrameIdx = Frame - 1;
	auto ShotIdx = Shot - 1;

	if (not FrameScores.IsValidIndex(FrameIdx)) { return false; }
	auto& CurrentFrame = FrameScores[FrameIdx];
	if (not CurrentFrame.Shots.IsValidIndex(ShotIdx)) { return false; }

	// First shot can't be a spare
	if (Shot == 1) { return false; }

	// Frame 10 must have a first strike for shot 3 to be a spare (XN/)
	if (Frame == 10 and Shot == 3 and CurrentFrame.Shots[0] != 10) { return false; }

	// Previous shot must not have been a strike and all pins must have been knocked down over the two shots (so X0 isn't detected as X/)
	if (CurrentFrame.Shots[ShotIdx - 1] < 10 and (CurrentFrame.Shots[ShotIdx - 1] + CurrentFrame.Shots[ShotIdx]) == 10)
	{
		return true;
	}

	return false;
}

bool UBowlingScoreComponent::IsStrike(int32 Frame, int32 Shot) const
{
	auto FrameIdx = Frame - 1;
	auto ShotIdx = Shot - 1;

	if (not FrameScores.IsValidIndex(FrameIdx)) { return false; }
	auto& CurrentFrame = FrameScores[FrameIdx];
	if (not CurrentFrame.Shots.IsValidIndex(ShotIdx)) { return false; }

	// If it's not all pins it can't be a strike
	if (CurrentFrame.Shots[ShotIdx] != 10) { return false; }

	// Strike is always possible on the first shot of any frame
	if (Shot == 1) { return true; }

	if (Frame == 10)
	{
		// First shot must be a strike for the second shot to be a strike instead of a spare
		if (Shot == 2 and CurrentFrame.Shots[0] == 10) { return true; }

		// XXX and N/X are the two possibilities for a shot 3 strike on frame 10
		// XXX case
		if (Shot == 3 and CurrentFrame.Shots[0] == 10 and CurrentFrame.Shots[1] == 10) { return true; }
		// N/X case, also make sure this isn't X0/
		if (Shot == 3 and CurrentFrame.Shots[0] < 10 and CurrentFrame.Shots[0] + CurrentFrame.Shots[1] == 10)
		{
			return true;
		}
	}

	return false;
}

bool UBowlingScoreComponent::IsGameOver() const
{
	return CurrentFrameIndex < 0 or CurrentFrameIndex > 9;
}

TPair<int32, int32> UBowlingScoreComponent::GetNextScoredShot(int32 Frame, int32 Shot) const
{
	// Thankfully this will never touch frame 10 shot 3
	auto FrameIdx = Frame - 1;
	auto ShotIdx = Shot - 1;

	auto NextShot = (ShotIdx + 1) % 2 + 1;
	auto NextFrame = FrameIdx + (ShotIdx + 1) / 2 + 1;

	TPair<int32, int32> PotentialNext = {NextFrame, NextShot};

	// Unless it's frame 10, skip the shot after a strike...
	if (NextFrame != 10 and NextShot == 2 and IsStrike(NextFrame, 1))
	{
		PotentialNext = GetNextScoredShot(NextFrame, NextShot);
	}

	return PotentialNext;
}

void UBowlingScoreComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	Reset();
}

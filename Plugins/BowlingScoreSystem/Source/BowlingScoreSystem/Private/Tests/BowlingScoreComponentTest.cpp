#include "BowlingScoreComponent.h"
#include "CQTest.h"
#include "Components/ActorTestSpawner.h"

TEST_CLASS(BowlingScoreTests, "Bowling.Score")
{
	FActorTestSpawner Spawner;
	UBowlingScoreComponent* Bowling;
	
	BEFORE_EACH()
	{
		Spawner = FActorTestSpawner();
		Bowling = &Spawner.SpawnObject<UBowlingScoreComponent>();
	}
	
	TEST_METHOD(BowlingScore_Initialize)
	{
		ASSERT_THAT(AreEqual(1, Bowling->GetCurrentFrameNum()));
		ASSERT_THAT(AreEqual(1, Bowling->GetCurrentShotNum()));

		ASSERT_THAT(IsFalse(Bowling->IsGameOver()));
	}

	TEST_METHOD(BowlingScore_ScoreInitialize)
	{
		for (auto Frame = 1; Frame <= 10; Frame++)
		{
			auto FrameScore = Bowling->GetFrameScore(Frame);
			ASSERT_THAT(AreEqual(0, FrameScore,
				FString::Format(TEXT("Frame score for Frame {0} should be 0 instead of {1}"), {Frame, FrameScore})));
			auto Score = Bowling->GetScore(Frame);
			ASSERT_THAT(AreEqual(0, Score,
				FString::Format(TEXT("Score at Frame {0} should be 0 instead of {1}"), {Frame, Score})));
			auto NumShots = Frame != 10 ? 2 : 3;
			for (auto Shot = 1; Shot <= NumShots; Shot++)
			{
				auto ShotScore = Bowling->GetShotScore(Frame, Shot);
				ASSERT_THAT(AreEqual(0, Score,
					FString::Format(TEXT("Score at Frame {0} Shot {1} should be 0 instead of {2}"), {Frame, Shot, ShotScore})));
			}
		}
		ASSERT_THAT(AreEqual(Bowling->GetScore(10), 0));
	}

	// On initialize, nothing should be reported as a spare or strike
	TEST_METHOD(BowlingScore_ShotInitialize)
	{
		for (auto Frame = 1; Frame <= 10; Frame++)
		{
			auto NumShots = Frame != 10 ? 2 : 3;
			for (auto Shot = 1; Shot <= NumShots; Shot++)
			{
				ASSERT_THAT(IsFalse(Bowling->IsStrike(Frame, Shot),
					FString::Format(TEXT("Frame {0} Shot {1} should not be a strike"), {Frame, Shot})));
				ASSERT_THAT(IsFalse(Bowling->IsSpare(Frame, Shot),
					FString::Format(TEXT("Frame {0} Shot {1} should not be a spare"), {Frame, Shot})));
			}
		}
	}

	// Test that state and score reset
	TEST_METHOD(BowlingScore_Reset)
	{
		for (auto i = 0; i < 20; i++)
		{
			Bowling->SetScore(1);
		}

		ASSERT_THAT(IsTrue(Bowling->IsGameOver()));
		ASSERT_THAT(AreEqual(-1, Bowling->GetCurrentFrameNum()));
		ASSERT_THAT(AreEqual(-1, Bowling->GetCurrentShotNum()));
		ASSERT_THAT(AreEqual(20, Bowling->GetScore(10)));
		
		Bowling->Reset();
		
		ASSERT_THAT(IsFalse(Bowling->IsGameOver()));
		ASSERT_THAT(AreEqual(1, Bowling->GetCurrentFrameNum()));
		ASSERT_THAT(AreEqual(1, Bowling->GetCurrentShotNum()));
		ASSERT_THAT(AreEqual(0, Bowling->GetScore(10)));
	}

	TEST_METHOD(BowlingScore_GetCurrentFrameNum)
	{
		// First 9
		for (auto i = 0; i < 18; i++)
		{
			auto ExpectedFrameNum = (i / 2) + 1;
			auto ExpectedShotNum = (i % 2) + 1;
			auto CurrentFrameNum = Bowling->GetCurrentFrameNum();
			ASSERT_THAT(AreEqual(ExpectedFrameNum, CurrentFrameNum,
				FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}"),
					{ExpectedFrameNum, ExpectedShotNum, ExpectedFrameNum, CurrentFrameNum})));
			
			Bowling->SetScore(1);
		}

		// Frame 10
		for (auto i = 0; i < 3; i++)
		{
			auto CurrentFrameNum = Bowling->GetCurrentFrameNum();
			ASSERT_THAT(AreEqual(10, CurrentFrameNum,
				FString::Format(TEXT("Frame 10 Shot {0}: Expected {1} to equal {2}"),
					{i+1, 10, CurrentFrameNum})));

			Bowling->SetScore(10);
		}

		ASSERT_THAT(AreEqual(-1, Bowling->GetCurrentFrameNum()));
	}

	TEST_METHOD(Bowling_GetCurrentShotNum)
	{
		// First 9
		for (auto i = 0; i < 18; i++)
		{
			auto ExpectedFrameNum = (i / 2) + 1;
			auto ExpectedShotNum = (i % 2) + 1;
			auto CurrentShotNum = Bowling->GetCurrentShotNum();
			ASSERT_THAT(AreEqual(ExpectedShotNum, CurrentShotNum,
				FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}"),
					{ExpectedFrameNum, ExpectedShotNum, ExpectedShotNum, CurrentShotNum})));
			
			Bowling->SetScore(1);
		}

		// Frame 10
		for (auto i = 0; i < 3; i++)
		{
			auto ExpectedShotNum = i + 1;
			auto CurrentShotNum = Bowling->GetCurrentShotNum();
			ASSERT_THAT(AreEqual(ExpectedShotNum, CurrentShotNum,
				FString::Format(TEXT("Frame 10 Shot {0}: Expected {1} to equal {2}"),
					{i+1, ExpectedShotNum, CurrentShotNum})));

			Bowling->SetScore(10);
		}

		ASSERT_THAT(AreEqual(-1, Bowling->GetCurrentShotNum()));
	}

	TEST_METHOD(Bowling_IsSpare)
	{
		Bowling->SetScore(10);
		ASSERT_THAT(IsFalse(Bowling->IsSpare(1, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsSpare(1, 2)));
		
		Bowling->SetScore(1);
		Bowling->SetScore(1);
		ASSERT_THAT(IsFalse(Bowling->IsSpare(2, 2)));

		Bowling->SetScore(0);
		Bowling->SetScore(10);
		ASSERT_THAT(IsTrue(Bowling->IsSpare(3, 2)));

		Bowling->SetScore(9);
		Bowling->SetScore(1);
		ASSERT_THAT(IsTrue(Bowling->IsSpare(4, 2)));

		// Possible frame 10 states:
		// XXX, XXN, XNN, XN/
		// N/X, N/N
		// NN
		
		// For the following tests, set time to frame 10
		// N/X
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;
		
		Bowling->SetScore(0);
		Bowling->SetScore(10);
		Bowling->SetScore(10);
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 1)));
		ASSERT_THAT(IsTrue(Bowling->IsSpare(10, 2)));
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 3)));

		// N/N
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;

		Bowling->SetScore(5);
		Bowling->SetScore(5);
		Bowling->SetScore(5);
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 1)));
		ASSERT_THAT(IsTrue(Bowling->IsSpare(10, 2)));
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 3)));

		// XXN
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;
		
		Bowling->SetScore(10);
		Bowling->SetScore(10);
		Bowling->SetScore(0);
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 2)));
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 3)));

		// XN/
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;
		
		Bowling->SetScore(10);
		Bowling->SetScore(6);
		Bowling->SetScore(4);
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsSpare(10, 2)));
		ASSERT_THAT(IsTrue(Bowling->IsSpare(10, 3)));
	}

	TEST_METHOD(BowlingScore_IsStrike)
	{
		Bowling->SetScore(0);
		Bowling->SetScore(10);
		ASSERT_THAT(IsFalse(Bowling->IsStrike(1, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsStrike(1, 2)));

		Bowling->SetScore(10);
		ASSERT_THAT(IsTrue(Bowling->IsStrike(2, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsStrike(2, 2)));

		Bowling->SetScore(5);
		Bowling->SetScore(5);
		ASSERT_THAT(IsFalse(Bowling->IsStrike(3, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsStrike(3, 2)));

		// Possible frame 10 states:
		// XXX, XXN, XNN, XN/
		// N/X, N/N
		// NN
		
		// For the following tests, set time to frame 10
		// XXX
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;

		Bowling->SetScore(10);
		Bowling->SetScore(10);
		Bowling->SetScore(10);
		ASSERT_THAT(IsTrue(Bowling->IsStrike(10, 1)));
		ASSERT_THAT(IsTrue(Bowling->IsStrike(10, 2)));
		ASSERT_THAT(IsTrue(Bowling->IsStrike(10, 3)));

		// N/X
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;

		Bowling->SetScore(0);
		Bowling->SetScore(10);
		Bowling->SetScore(10);
		ASSERT_THAT(IsFalse(Bowling->IsStrike(10, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsStrike(10, 2)));
		ASSERT_THAT(IsTrue(Bowling->IsStrike(10, 3)));

		// N/X
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;

		Bowling->SetScore(5);
		Bowling->SetScore(5);
		Bowling->SetScore(10);
		ASSERT_THAT(IsFalse(Bowling->IsStrike(10, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsStrike(10, 2)));
		ASSERT_THAT(IsTrue(Bowling->IsStrike(10, 3)));
	}

	TEST_METHOD(BowlingScore_IsGameOver)
	{
		for (auto i = 0; i < 20; i++)
		{
			ASSERT_THAT(IsFalse(Bowling->IsGameOver(),
				FString::Format(TEXT("Frame {0} Shot {1}: Expected false"), {(i/2)+1, (i%2)+1})))
			Bowling->SetScore(1);
		}
		
		ASSERT_THAT(IsTrue(Bowling->IsGameOver()));
	}
	
	TEST_METHOD(BowlingScore_IsGameOver_FullFrame10)
	{
		for (auto i = 0; i < 9; i++)
		{
			ASSERT_THAT(IsFalse(Bowling->IsGameOver(),
				FString::Format(TEXT("Frame {0} Shot {1}: Expected false"), {(i/2)+1, (i%2)+1})))
			Bowling->SetScore(10);
		}

		for (auto i = 0; i < 3; i++)
		{
			ASSERT_THAT(IsFalse(Bowling->IsGameOver(),
	FString::Format(TEXT("Frame 10 Shot {1}: Expected false"), {(i%2)+1})))
			Bowling->SetScore(10);
		}
		
		ASSERT_THAT(IsTrue(Bowling->IsGameOver()));
	}

	TEST_METHOD(BowlingScore_GetShotScore)
	{
		for (auto i = 0; i < 21; i++)
		{
			auto Frame = Bowling->GetCurrentFrameNum();
			auto Shot = Bowling->GetCurrentShotNum();

			auto ShotScore = Bowling->GetShotScore(Frame, Shot);
			ASSERT_THAT(AreEqual(0, ShotScore,
				FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}"), {Frame, Shot, 0, ShotScore})));
			Bowling->SetScore(5);
			ShotScore = Bowling->GetShotScore(Frame, Shot);
			ASSERT_THAT(AreEqual(5, ShotScore,
				FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}"), {Frame, Shot, 5, ShotScore})));
		}
	}
	
	TEST_METHOD(BowlingScore_GetFrameScore)
	{
	    // Frame 1
		Bowling->SetScore(5);
		Bowling->SetScore(4);
		ASSERT_THAT(AreEqual(9, Bowling->GetFrameScore(1)));

		// Frame 2
		Bowling->SetScore(5);
		Bowling->SetScore(5);
		ASSERT_THAT(AreEqual(10, Bowling->GetFrameScore(2)));

		// Frame 3
		Bowling->SetScore(4);
		Bowling->SetScore(3);
		ASSERT_THAT(AreEqual(14, Bowling->GetFrameScore(2)));

		// Frame 4
		Bowling->SetScore(10);
		ASSERT_THAT(AreEqual(10, Bowling->GetFrameScore(4)));

		// Frame 5
		Bowling->SetScore(10);
		ASSERT_THAT(AreEqual(20, Bowling->GetFrameScore(4)));

		// Frame 6
		Bowling->SetScore(10);
		ASSERT_THAT(AreEqual(30, Bowling->GetFrameScore(4)));

		// Frame 7
		Bowling->SetScore(5);
		ASSERT_THAT(AreEqual(15, Bowling->GetFrameScore(6)));
		Bowling->SetScore(4);
		ASSERT_THAT(AreEqual(19, Bowling->GetFrameScore(6)));

		// For the next checks, set state to frame 10
		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;

		Bowling->SetScore(10);
		ASSERT_THAT(AreEqual(10, Bowling->GetFrameScore(10)));
		Bowling->SetScore(10);
		ASSERT_THAT(AreEqual(20, Bowling->GetFrameScore(10)));
		Bowling->SetScore(10);
		ASSERT_THAT(AreEqual(30, Bowling->GetFrameScore(10)));

		Bowling->CurrentFrameIndex = 9;
		Bowling->CurrentShotIndex = 0;
		Bowling->FrameScores[9] = {0, 0, 0};
		Bowling->SetScore(5);
		ASSERT_THAT(AreEqual(5, Bowling->GetFrameScore(10)));
		Bowling->SetScore(5);
		ASSERT_THAT(AreEqual(10, Bowling->GetFrameScore(10)));
		Bowling->SetScore(5);
		ASSERT_THAT(AreEqual(15, Bowling->GetFrameScore(10)));
	}

	TEST_METHOD(BowlingScore_GetScore)
	{
		// Just check that it's accumulating score
		for (auto i = 0; i < 21; i++)
		{
			auto CurrentFrame = Bowling->GetCurrentFrameNum();
			auto CurrentShot = Bowling->GetCurrentShotNum();
			
			Bowling->SetScore(5);
			auto ExpectedScore = 0;
			for (auto Frame = 1; Frame <= CurrentFrame; Frame++)
			{
				ExpectedScore += Bowling->GetFrameScore(Frame);
			}

			auto Score = Bowling->GetScore(CurrentFrame);

			ASSERT_THAT(AreEqual(ExpectedScore, Score,
				FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} equal to {3}"), {CurrentFrame, CurrentShot, ExpectedScore, Score})))
		}
	}

	TEST_METHOD(BowlingScore_IsValidShotScore)
	{
		// shot number
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(1, 1, 0)));
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(1, 1, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(1, 1, 3)));

		// frame number
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(1, 0, 1)));
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(1, 1, 1)));
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(1, 11, 1)));

		// score
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(-1)));
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(0)));
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(10)));
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(11)));

		// if it's valid, then it should be allowed to be set as well
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(5)));
		ASSERT_THAT(IsTrue(Bowling->SetScore(5)));
		
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(4)));
		// And if it's not valid, it shouldn't be allowed to be set
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(6)));
		ASSERT_THAT(IsFalse(Bowling->SetScore(6)));

		// Spare
		Bowling->Reset();
		Bowling->SetScore(5);
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(5)));
		ASSERT_THAT(IsTrue(Bowling->SetScore(5)));
		
		// strike
		Bowling->Reset();
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(10)));
		Bowling->SetScore(10);
		// Check that even a 0 isn't valid as an entry after a strike in the frame
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(0, 1, 2)));
	}
	
	TEST_METHOD(BowlingScore_IsValidShotScore_FrameTen)
	{
		auto ResetToFrameTen = [this]()
		{
			Bowling->Reset();
			Bowling->CurrentFrameIndex = 9;
			Bowling->CurrentShotIndex = 0;
		};
		
		// NN
		ResetToFrameTen();
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(1)));
		Bowling->SetScore(1);
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(1)));
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(10)));
		Bowling->SetScore(1);
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(1)));

		// N/N N/X
		ResetToFrameTen();
		Bowling->SetScore(1);
		Bowling->SetScore(9);
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(10)));
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(1)));
		Bowling->SetScore(10);
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(1)));

		// XXX, XXN
		ResetToFrameTen();
		Bowling->SetScore(10);
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(10)));
		Bowling->SetScore(10);
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(10)));
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(1)));
		
		// XNN, XN/
		ResetToFrameTen();
		Bowling->SetScore(10);
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(5)));
		Bowling->SetScore(5);
		ASSERT_THAT(IsTrue(Bowling->IsValidShotScore(4)));
		ASSERT_THAT(IsFalse(Bowling->IsValidShotScore(6)));
	}

	TEST_METHOD(BowlingScore_SetScore)
	{
		// This has been tested pretty exhaustively in other tests so check the return value expectations

		ASSERT_THAT(IsFalse(Bowling->SetScore(11)));
		ASSERT_THAT(IsFalse(Bowling->SetScore(-1)));

		// Check that no state was actually changed
		ASSERT_THAT(AreEqual(1, Bowling->GetCurrentFrameNum()));
		ASSERT_THAT(AreEqual(1, Bowling->GetCurrentShotNum()));
		ASSERT_THAT(AreEqual(0, Bowling->GetShotScore(1, 1)));
		ASSERT_THAT(IsTrue(Bowling->SetScore(3)));

		// State should finally have been changed
		ASSERT_THAT(AreEqual(1, Bowling->GetCurrentFrameNum()));
		ASSERT_THAT(AreEqual(2, Bowling->GetCurrentShotNum()));
		ASSERT_THAT(AreEqual(3, Bowling->GetShotScore(1, 1)));
	}
	
	// Test the example game from the instructions document. Check strikes, spares, and scores.
	TEST_METHOD(BowlingScore_ExampleGame_SetState)
	{
		Bowling->FrameScores = {
			{8, 2}, {5, 4}, {9, 0}, {10, 0}, {10, 0}, {5, 5}, {5, 3}, {6, 3}, {9, 1}, {9, 1, 10}
		};
		Bowling->CurrentFrameIndex = 10;

		TArray<int32> FrameScores = {15, 9, 9, 25, 20, 15, 8, 9, 19, 20};
		TArray<int32> CumulativeScores;
		auto Sum = 0;
		for (auto&& Score : FrameScores)
		{
			Sum += Score;
			CumulativeScores.Push(Sum);
		}
		
		TArray<TPair<int32, int32>> SpareShots = {{1,2}, {6,2}, {9,2}, {10,2}};
		TArray<TPair<int32, int32>> StrikeShots = {{4,1}, {5,1}, {10, 3}};
		
		for (auto Frame = 1; Frame <= 10; Frame++)
		{
			auto FrameScore = Bowling->GetFrameScore(Frame);
			auto CumulativeScore = Bowling->GetScore(Frame);
			ASSERT_THAT(AreEqual(FrameScores[Frame-1], FrameScore,
				FString::Format(TEXT("Frame {0}: Expected {1} to equal {2}."),
					{Frame, FrameScores[Frame-1], FrameScore})));
			ASSERT_THAT(AreEqual(CumulativeScores[Frame-1], CumulativeScore,
				FString::Format(TEXT("Frame {0}: Expected {1} to equal {2}."),
					{Frame, CumulativeScores[Frame-1], CumulativeScore})));
			auto NumShots = Frame != 10 ? 2 : 3;
			for (auto Shot = 1; Shot <= NumShots; Shot++)
			{
				auto IsSpare = Bowling->IsSpare(Frame, Shot);
				auto ExpectedSpare = SpareShots.Contains(TPair<int32, int32>{Frame, Shot});
				auto IsStrike = Bowling->IsStrike(Frame, Shot);
				auto ExpectedStrike = StrikeShots.Contains(TPair<int32, int32>{Frame, Shot});
				ASSERT_THAT(AreEqual( ExpectedSpare, IsSpare,
					FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}."),
						{Frame, Shot, ExpectedSpare, IsSpare})));
				ASSERT_THAT(AreEqual( ExpectedStrike, IsStrike,
					FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}."),
						{Frame, Shot, ExpectedStrike, IsStrike})));
			}
		}

		ASSERT_THAT(IsTrue(Bowling->IsGameOver()));
		Bowling->Reset();
		ASSERT_THAT(AreEqual(Bowling->GetScore(10), 0));
		ASSERT_THAT(IsFalse(Bowling->IsGameOver()));
	}

#define SET_SCORE(Score) \
	ASSERT_THAT(IsTrue(Bowling->SetScore(Score)));
#define CHECK_FRAME_SCORE(Frame, ExpectedScore) \
	{\
		auto FrameScore = Bowling->GetFrameScore(Frame); \
		ASSERT_THAT(AreEqual(ExpectedScore, FrameScore, FString::Format(TEXT("Frame {0}: Expected {1} to equal {2}."), {Frame, ExpectedScore, FrameScore}))); \
	}
#define CHECK_FRAME(Frame) \
	{\
	    ASSERT_THAT(AreEqual(Frame, Bowling->GetCurrentFrameNum()));\
	}
	
	// Test the example game from the document shot by shot using the external interface
	TEST_METHOD(BowlingScore_ExampleGame)
	{
		ASSERT_THAT(IsFalse(Bowling->IsGameOver()));
		
		// Frame 1
		CHECK_FRAME(1);
		SET_SCORE(8);
		CHECK_FRAME_SCORE(1, 8);
		SET_SCORE(2);
		CHECK_FRAME_SCORE(1, 10);
		ASSERT_THAT(IsTrue(Bowling->IsSpare(1, 2)));

		// Frame 2
		CHECK_FRAME(2);
		SET_SCORE(5);
		CHECK_FRAME_SCORE(2, 5);
		CHECK_FRAME_SCORE(1, 15);
		SET_SCORE(4);
		CHECK_FRAME_SCORE(2, 9);
		
		// Frame 3
		CHECK_FRAME(3);
		SET_SCORE(9);
		CHECK_FRAME_SCORE(3, 9);
		SET_SCORE(0);
		CHECK_FRAME_SCORE(3, 9);

		// Frame 4
		CHECK_FRAME(4);
		SET_SCORE(10);
		CHECK_FRAME_SCORE(4, 10);

		// Frame 5
		CHECK_FRAME(5);
		SET_SCORE(10);
		CHECK_FRAME_SCORE(5, 10);
		CHECK_FRAME_SCORE(4, 20);

		// Frame 6
		CHECK_FRAME(6);
		SET_SCORE(5);
		CHECK_FRAME_SCORE(5, 15);
		CHECK_FRAME_SCORE(6, 5);
		SET_SCORE(5);
		CHECK_FRAME_SCORE(5, 20);
		CHECK_FRAME_SCORE(6, 10);

		// Frame 7
		CHECK_FRAME(7);
		SET_SCORE(5);
		CHECK_FRAME_SCORE(6, 15);
		CHECK_FRAME_SCORE(7, 5);
		SET_SCORE(3);
		CHECK_FRAME_SCORE(7, 8);
		
		// Frame 8
		CHECK_FRAME(8);
		SET_SCORE(6);
		CHECK_FRAME_SCORE(8, 6);
		SET_SCORE(3);
		CHECK_FRAME_SCORE(8, 9);

		// Frame 9
		CHECK_FRAME(9);
		SET_SCORE(9);
		CHECK_FRAME_SCORE(9, 9);
		SET_SCORE(1);
		CHECK_FRAME_SCORE(9, 10);

		// Frame 10
		CHECK_FRAME(10);
		SET_SCORE(9);
		CHECK_FRAME_SCORE(9, 19);
		CHECK_FRAME_SCORE(10, 9);
		SET_SCORE(1);
		CHECK_FRAME_SCORE(10, 10);
		SET_SCORE(10);
		CHECK_FRAME_SCORE(10, 20);
		
		// End of game checks
		ASSERT_THAT(IsTrue(Bowling->IsGameOver()));
		ASSERT_THAT(AreEqual(-1, Bowling->GetCurrentFrameNum()));
		ASSERT_THAT(AreEqual(-1, Bowling->GetCurrentShotNum()));
		auto TotalScore = Bowling->GetScore(10);
		ASSERT_THAT(AreEqual(149, TotalScore));
	}
};

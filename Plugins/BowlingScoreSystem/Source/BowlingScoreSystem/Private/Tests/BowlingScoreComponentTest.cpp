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
				FString::Format(TEXT("Score for Frame {0} should be 0 instead of {1}"), {Frame, FrameScore})));
		}
		ASSERT_THAT(AreEqual(Bowling->GetScore(10), 0));
	}

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

	// Test the example game from the instructions document. Check strikes, spares, and scores.
	TEST_METHOD(BowlingScore_ExampleGame_SetState)
	{
		Bowling->FrameScores = TArray<FBowlingFrameScore>{
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
				FString::Format(TEXT("Frame {0}: Expected {1} to equal {2}."), {Frame, FrameScores[Frame-1], FrameScore})));
			ASSERT_THAT(AreEqual(CumulativeScores[Frame-1], CumulativeScore,
				FString::Format(TEXT("Frame {0}: Expected {1} to equal {2}."), {Frame, CumulativeScores[Frame-1], CumulativeScore})));
			auto NumShots = Frame != 10 ? 2 : 3;
			for (auto Shot = 1; Shot <= NumShots; Shot++)
			{
				auto IsSpare = Bowling->IsSpare(Frame, Shot);
				auto ExpectedSpare = SpareShots.Contains(TPair<int32, int32>{Frame, Shot});
				auto IsStrike = Bowling->IsStrike(Frame, Shot);
				auto ExpectedStrike = StrikeShots.Contains(TPair<int32, int32>{Frame, Shot});
				ASSERT_THAT(AreEqual( ExpectedSpare, IsSpare,
					FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}."), {Frame, Shot, ExpectedSpare, IsSpare})));
				ASSERT_THAT(AreEqual( ExpectedStrike, IsStrike,
					FString::Format(TEXT("Frame {0} Shot {1}: Expected {2} to equal {3}."), {Frame, Shot, ExpectedStrike, IsStrike})));
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

// TODO: Add more tests
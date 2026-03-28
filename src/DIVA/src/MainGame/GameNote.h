#pragma once
#include "Common/Types.h"
#include "TimeSpan.h"
#include "Chart.h"
#include "HitEvaluation.h"
#include "MainGame.h"

using Starshine::TimeSpan;
using Starshine::GameTime;

namespace DIVA::MainGame
{
	constexpr vec2 DefaultIconPosition{ std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity() };

	struct GameNote
	{
	public:
		GameNote() {};
		GameNote(const ChartNote& chartNote, MainGame::MainGameContext& context) :
			Shape(chartNote.Shape),
			Type(chartNote.Type),
			TargetPosition(chartNote.X, chartNote.Y),
			EntryAngle(chartNote.Angle),
			Frequency(chartNote.Frequency),
			Amplitude(chartNote.Amplitude),
			Distance(chartNote.Distance),
			MainGameContext(&context)
		{};

	public:
		MainGameContext* MainGameContext{};

	public:
		TimeSpan FlyTime{};
		TimeSpan ElapsedTime{};

		TimeSpan RemainingTimeOnHit{};

		NoteShape Shape{};
		NoteType Type{};
		vec2 TargetPosition{};
		vec2 IconPosition{ DefaultIconPosition };

		float EntryAngle{};
		float Frequency{};
		float Amplitude{};
		float Distance{};

		bool ActiveDuringChanceTime{};

	public:
		// NOTE: Generic state flags
		bool Expiring = false;
		bool Expired = false;
		bool ShouldBeRemoved = false;

	public:
		// NOTE: Hit Stats
		HitEvaluation HitEvaluation{ HitEvaluation::None };
		bool HasBeenHit = false;
		bool HitWrong = false;

		struct DoubleTapData
		{
			bool Primary = false;
			bool Alternative = false;

			bool GiveBonus = true;
		} DoubleTap;

		struct HoldData
		{
			bool PrimaryHeld = false;
			bool AlternativeHeld = false;

			TimeSpan TimeSinceHoldStart{};

			i32 BonusBaseValue{};
			i32 CurrentBonus{};
		} Hold;

	public:
		GameNote* NextNote{ nullptr };

	public:
		TimeSpan GetRemainingTime() const;

		f64 GetNormalizedElapsedTime() const;
		f64 GetNormalizedRemainingTime() const;

		bool HasBeenEvaluated() const;

	public:
		struct TrailData
		{
			f32 Start{};
			f32 End{};

			f32 Max{ 1.0f };

			f32 Scroll{};
			f32 ScrollResetThreshold{};

			bool Hold{ false };
		} Trail;

		void UpdateTrail();
		void DrawTrail();

	public:
		void Update(GameTime& gameTime);
		void Draw(GameTime& gameTime);

		bool Evaluate(NoteShape shape);
	};
}

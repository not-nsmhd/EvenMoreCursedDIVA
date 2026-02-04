#pragma once
#include "Common/Types.h"
#include "Chart.h"
#include "HitEvaluation.h"
#include "MainGame.h"

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
		f64 FlyTime{};
		f64 ElapsedTime{};

		f64 RemainingTimeOnHit{};

		NoteShape Shape{};
		NoteType Type{};
		vec2 TargetPosition{};
		vec2 IconPosition{ DefaultIconPosition };

		float EntryAngle{};
		float Frequency{};
		float Amplitude{};
		float Distance{};

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

			f64 TimeSinceHoldStart{};

			i32 CurrentBonus{};
		} Hold;

	public:
		GameNote* NextNote{ nullptr };

	public:
		// NOTE: Functions
		// NOTE: Returned value is specified in seconds
		f64 GetRemainingTime() const;
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

		void DrawTrail();

	public:
		void Update(f64 deltaTime_ms);
		void Draw(f64 deltaTime_ms);

		bool Evaluate(NoteShape shape);
	};
}

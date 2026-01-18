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

	public:
		// NOTE: Functions
		// NOTE: Returned value is specified in seconds
		f64 GetRemainingTime() const;
		f64 GetNormalizedElapsedTime() const;

		bool HasBeenEvaluated() const;

	public:
		void Update(f64 deltaTime_ms);
		void Draw(f64 deltaTime_ms);

		bool Evaluate(NoteShape shape, bool ignoreWrong);

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
	};
}

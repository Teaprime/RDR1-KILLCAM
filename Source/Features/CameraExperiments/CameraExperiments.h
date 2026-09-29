#pragma once

class CameraExperiments
{
public:
	static void Update();
	static void OnActorKilled(int actorId);
	static void OnSecondStage();

	static inline float s_Timescale = 0.20;
	static inline float s_CutBackTimescale = 0.25f;
	static inline float s_CutBackChance = 0.5f;
	static inline float s_FirstshotDuration = 0.3f;
	static inline float s_Duration = 0.5f;
	//Whether to always track the target after camera switch
	static inline bool s_doFollowCam = false;

private:
	static int FindTargetActor();

	static inline Camera s_ExperimentCam = 0;
	static inline bool s_Active = false;
	static inline bool s_SecondStageActive = false;
	static inline Actor s_TargetActor = -1;
	static inline bool s_WasOnFoot = true;
	static inline bool s_PlayerWasOnFoot = true;
	static inline bool s_WillCutBack = false;

	// timing
	static inline float s_Timer = 0.0f;

	// Timescale
	static inline float s_OrigTimeScale = 1.0f;


	// camera placement
	static inline Vector2 s_Distance = { 1.0f, 3.0f };
	static inline Vector2 s_DistanceNonFoot = { 3.0f, 5.0f };
	static inline Vector2 s_VertOffset = { -0.5f, 0.6f };
	static inline Vector2 s_VertOffsetNonFoot = { -0.1f, 0.3f };
	static inline Vector3 s_ActorCenterOffset = { 0.0f, 1.3f, 0.0f };

	static inline Vector3 s_ActorCenterOffsetNonFoot = { 0.0f, 0.5f, 0.0f };

	static inline float s_CutBackHeight = 1.3f;
	static inline float s_CutBackFrontAngleMax = 10.0f; // degrees
	static inline float s_CutBackDistance = 2.0f;
	static inline float s_CutBackDistanceNonFoot = 3.0f;


	//Camera FOV
	static inline float s_FOV = 50.0f;
	static inline float s_CutBackFOV = 30.0f;

	//Choose whether to always track target after camera switch


	// layout
	static inline Layout s_PlayerLayout = 0;

	static inline float s_WasObstructed = false;
};
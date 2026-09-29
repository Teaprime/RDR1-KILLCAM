#include <pch.h>
static Vector3 GET_FORWARD_VECTOR(Actor _Actor)
{
	Vector3 forward;
	float heading = ACTOR::GET_HEADING(_Actor) + 90.0f;

	heading *= Math<float>::DegToRad;

	forward.x = std::cos(heading) * -1.0f;
	forward.y = 0.0f;
	forward.z = std::sin(heading);

	return forward;
}

static Vector3 GET_RIGHT_VECTOR(Actor _Actor)
{
	Vector3 right;
	float heading = ACTOR::GET_HEADING(_Actor) + 180.0f;

	heading *= Math<float>::DegToRad;

	right.x = std::cos(heading) * -1.0f;
	right.y = 0.0f;
	right.z = std::sin(heading);

	return right;
}

int CameraExperiments::FindTargetActor()
{
	constexpr int MAX_ACTORS = 256;
	int actors[MAX_ACTORS];
	int count = REDHOOK::WORLD_GET_ALL_ACTORS(actors);

	for (int i = 0; i < count; ++i)
	{
		int a = actors[i];
		if (!ENTITY::IS_ACTOR_VALID(a)) continue;
		if (ACTOR::IS_ACTOR_LOCAL_PLAYER(a)) continue;

		return a;
	}

	return -1;
}

void CameraExperiments::OnActorKilled(int actorId)
{
	// If already active, ignore new requests
	if (s_Active) return;
	if (!ENTITY::IS_ACTOR_VALID(actorId)) return;

	s_TargetActor = actorId;
	Vector3 actorPos = ACTOR::GET_POSITION(s_TargetActor);
	s_WasOnFoot = ENTITY::IS_ACTOR_ON_FOOT(actorId);

	// Attempt to get a layout. Using PlayerLayout as a safe fallback since
	// there is no obvious native to get an actor-specific layout.
	s_PlayerLayout = OBJECT::FIND_NAMED_LAYOUT("PlayerLayout");

	// Create a camera in layout
	s_ExperimentCam = CAMERA::CREATE_CAMERA_IN_LAYOUT(s_PlayerLayout, "camera_experiment", 0);
	if (!s_ExperimentCam)
	{
		s_TargetActor = -1;
		return;
	}

	// Initialize from current game camera
	CAMERA::INIT_CAMERA_FROM_GAME_CAMERA(s_ExperimentCam);

	// choose a random angle & distance around the actor
	float angleDeg = CORE::RAND_FLOAT_RANGE(0.0f, 360.0f);
	float angleRad = angleDeg * Math<float>::DegToRad;
	float dist = CORE::RAND_FLOAT_RANGE(s_Distance.x, s_Distance.y);
	float zOff = CORE::RAND_FLOAT_RANGE(s_VertOffset.x, s_VertOffset.y);
	if (!s_WasOnFoot) {
		zOff = CORE::RAND_FLOAT_RANGE(s_VertOffsetNonFoot.x, s_VertOffsetNonFoot.y);
		dist = CORE::RAND_FLOAT_RANGE(s_DistanceNonFoot.x, s_DistanceNonFoot.y);
	}

	actorPos += s_WasOnFoot ? s_ActorCenterOffset : s_ActorCenterOffsetNonFoot; // aim at the actor's center

	Vector3 camPos;
	camPos.x = actorPos.x + std::cos(angleRad) * dist;
	camPos.z = actorPos.z + std::sin(angleRad) * dist;
	camPos.y = actorPos.y + zOff;

	// set camera position and point at the actor
	CAMERA::SET_CAMERA_POSITION(s_ExperimentCam, PACK_VECTOR3(camPos));
	Vector3 dir = actorPos - camPos;
	CAMERA::SET_CAMERA_DIRECTION(s_ExperimentCam, PACK_VECTOR3(dir), TRUE);
	//Tighter framing with FOV adjustment
	CAMERA::SET_CAMERA_FOV(s_ExperimentCam, s_FOV);

	// activate camera (default channel params)
	CAMERA::SET_CURRENT_CAMERA_ON_CHANNEL(s_ExperimentCam, 0, 0, 0, 0, 0, 0, 0, 0, 0);


	// slow down time
	REDHOOK::SET_TIME_SCALE(s_Timescale);

	s_WillCutBack = CORE::RAND_FLOAT_RANGE(0.0f, 1.0f) <= s_CutBackChance;

	// start timer
	s_Timer = 0.0f;
	s_Active = true;

}

void CameraExperiments::OnSecondStage() {
	REDHOOK::SET_TIME_SCALE(s_CutBackTimescale);

	Actor playerActor = ACTOR::GET_PLAYER_ACTOR(-1);
	Vector3 actorPos = ACTOR::GET_POSITION(playerActor);
	//actorPos.y += s_CutBackHeight; // aim at the actor's center
	actorPos += s_PlayerWasOnFoot ? s_ActorCenterOffset : s_ActorCenterOffsetNonFoot; // aim at the actor's center

	Vector3 forwardVector = GET_FORWARD_VECTOR(playerActor);
	
	// choose a random angle & distance around the actor
	float angleDeg = CORE::RAND_FLOAT_RANGE(-s_CutBackFrontAngleMax, s_CutBackFrontAngleMax);
	float angleRad = angleDeg * Math<float>::DegToRad;
	float zOff = CORE::RAND_FLOAT_RANGE(s_VertOffset.x, s_VertOffset.y);

	Vector3 camPos;
	float dist = s_PlayerWasOnFoot ? s_CutBackDistance : s_CutBackDistanceNonFoot;
	camPos = actorPos - (forwardVector.Rotate(angleRad) * s_CutBackDistance);
	camPos.y += zOff;

	// set camera position and point at the actor
	CAMERA::SET_CAMERA_POSITION(s_ExperimentCam, PACK_VECTOR3(camPos));
	Vector3 dir = actorPos - camPos;
	CAMERA::SET_CAMERA_DIRECTION(s_ExperimentCam, PACK_VECTOR3(dir), TRUE);
	//Tighter framing with FOV adjustment
	CAMERA::SET_CAMERA_FOV(s_ExperimentCam, s_CutBackFOV);
}

void CameraExperiments::Update()
{
	// keep updating active experiment camera
	if (!s_Active) return;

	// if target died/invalid, cleanup
	if (!ENTITY::IS_ACTOR_VALID(s_TargetActor) || !s_ExperimentCam)
	{
		// restore
		if (s_ExperimentCam)
		{
			CAMERA::REMOVE_CAMERA_FROM_CHANNEL(s_ExperimentCam, 0);
			s_ExperimentCam = 0;
		}

		CAM::SET_CAMERA_FOLLOW_ACTOR(ACTOR::GET_PLAYER_ACTOR(-1));
		REDHOOK::SET_TIME_SCALE(s_OrigTimeScale);
		s_TargetActor = -1;
		s_Active = false;
		s_SecondStageActive = false;
		s_Timer = 0.0f;
		return;
	}

	// Advance timer
	s_Timer += BUILTIN::TIMESTEP();

	if (s_Timer >= s_Duration)
	{
		// restore camera & time
		if (s_ExperimentCam)
		{
			CAMERA::REMOVE_CAMERA_FROM_CHANNEL(s_ExperimentCam, 0);
			s_ExperimentCam = 0;
		}

		CAM::SET_CAMERA_FOLLOW_ACTOR(ACTOR::GET_PLAYER_ACTOR(-1));
		REDHOOK::SET_TIME_SCALE(s_OrigTimeScale);

		s_TargetActor = -1;
		s_Active = false;
		s_SecondStageActive = false;
		s_Timer = 0.0f;
		return;
	}
	else if(s_WillCutBack && s_Timer >= s_FirstshotDuration && !s_SecondStageActive)
	{
		s_SecondStageActive = true;
		s_PlayerWasOnFoot = ENTITY::IS_ACTOR_ON_FOOT(ACTOR::GET_PLAYER_ACTOR(-1));
		OnSecondStage();
	}

	// keep camera pointing at the actor (simple immediate follow)
	if ((s_SecondStageActive && !s_PlayerWasOnFoot) || (!s_SecondStageActive && (s_doFollowCam || !s_WasOnFoot))) {
		Vector3 actorPos = s_SecondStageActive ? ACTOR::GET_POSITION(ACTOR::GET_PLAYER_ACTOR(-1)) : ACTOR::GET_POSITION(s_TargetActor);
		bool wasOnFoot = s_SecondStageActive ? s_PlayerWasOnFoot : s_WasOnFoot;
		actorPos += wasOnFoot ? s_ActorCenterOffset : s_ActorCenterOffsetNonFoot;
		

		// compute cam position relative to actor from current camera position
		Vector3 camPos = CAMERA::GET_CAMERA_POSITION(s_ExperimentCam);
		Vector3 dir = actorPos - camPos;
		CAMERA::SET_CAMERA_DIRECTION(s_ExperimentCam, PACK_VECTOR3(dir), TRUE);

		if(s_SecondStageActive) {
			// keep camera behind player
			Vector3 forwardVector = GET_FORWARD_VECTOR(ACTOR::GET_PLAYER_ACTOR(-1));
			float dist = s_PlayerWasOnFoot ? s_CutBackDistance : s_CutBackDistanceNonFoot;
			camPos = actorPos - (forwardVector * dist);
			CAMERA::SET_CAMERA_POSITION(s_ExperimentCam, PACK_VECTOR3(camPos));
		}
	}
}
#include <pch.h>

void KillCam::OnActorKilled(int actorId)
{
	// Classic killcam behaviour (unchanged)
	REDHOOK::SET_TIME_SCALE(0.25f); // Slow down time for dramatic effect
	Camera gameCam = CAM::GET_GAME_CAMERA();
	if (!gameCam) return;

	// Save original camera direction
	s_OrigCamDir = CAMERA::GET_CAMERA_DIRECTION(gameCam);
	s_OrigHeading = ACTOR::GET_HEADING(ACTOR::GET_PLAYER_ACTOR(ACTOR::GET_LOCAL_SLOT()));

	// Set up killcam state
	s_TargetActor = actorId;
	s_KillCamEnabled = true;
	s_KillCamTimer = 0.0f;

	// Focus camera on the killed actor
	CAM::SET_CAMERA_FOLLOW_ACTOR(actorId);

	Vector3 actorPos = ACTOR::GET_POSITION(actorId);
	Vector3 camPos = CAMERA::GET_CAMERA_POSITION(gameCam);

	Vector3 dir = actorPos - camPos;
	Vector2 dirXY = Vector2(dir.x, dir.y);
	CAMERA::SET_CAMERA_DIRECTION(gameCam, dirXY, dir.z, true);
}

void KillCam::UpdateActorKillDetection()
{
	constexpr int MAX_ACTORS = 70;
	int actors[MAX_ACTORS];
	int count = REDHOOK::WORLD_GET_ALL_ACTORS(actors);

	Actor localActor = ACTOR::GET_PLAYER_ACTOR(-1);
	if (!ENTITY::IS_ACTOR_VALID(localActor)) return;

	Actor killedActor = -1;
	int killedSpecies = SPECIES_HUMAN;
	int hostilesAmount = 0;

	for (int i = 0; i < count; ++i)
	{
		int a = actors[i];
		if (!ENTITY::IS_ACTOR_VALID(a)) continue;
		if (ACTOR::IS_ACTOR_LOCAL_PLAYER(a)) continue;

		if (HEALTH::IS_ACTOR_DEAD(a))
		{
			if (s_handledDead.find(a) != s_handledDead.end()) continue;

			Actor attackerAny = HEALTH::GET_LAST_ATTACKER(a);
			int attacker = (int)attackerAny;

			if (attacker == (int)localActor)
			{
				killedActor = a;
				killedSpecies = AI_ANIMAL::ANIMAL_ACTOR_GET_SPECIES(a);

				s_handledDead.insert(a);
			}
		}
		else
		{
			auto it = s_handledDead.find(a);
			if (it != s_handledDead.end()) s_handledDead.erase(it);
			hostilesAmount += AI_MISC::AI_IS_HOSTILE_OR_ENEMY(a, localActor);
		}
	}

	bool doKillCam = killedActor != -1 && (killedSpecies == -1 || killedSpecies == SPECIES_HORSE || killedSpecies == SPECIES_MULE) && ((hostilesAmount == 0 && s_DoOnFinalEnemy) || CORE::RAND_FLOAT_RANGE(0.0f, 1.0f) <= s_TriggerProbability);
	if (doKillCam) {
		CameraExperiments::OnActorKilled(killedActor);
	}

	/*std::string msg = std::format("Hostiles: {}", hostilesAmount);
	HUD::HUD_CLEAR_OBJECTIVE_QUEUE();
	HUD::PRINT_OBJECTIVE_B(msg.c_str(), 0.1f, true, 2, 1, 0, 0, 0);*/
}

void KillCam::Update()
{
	// detect kills first (native-backed detection)
	UpdateActorKillDetection();

	// toggle experimental mode with F9
	/*if (REDHOOK::IS_KEY_PRESSED(KEY_F9))
	{
		s_UseExperimental = !s_UseExperimental;
	}*/

	// handle active classic killcam timing
	if (s_KillCamEnabled)
	{
		s_KillCamTimer += BUILTIN::TIMESTEP();
		if (s_KillCamTimer >= s_KillCamDuration)
		{
			// restore camera and reset state
			s_KillCamEnabled = false;
			s_KillCamTimer = 0.0f;

			Actor playerActor = ACTOR::GET_PLAYER_ACTOR(ACTOR::GET_LOCAL_SLOT());
			ACTOR::SET_ACTOR_HEADING(playerActor, s_OrigHeading, false);
			CAM::SET_CAMERA_FOLLOW_ACTOR(playerActor);
			s_TargetActor = playerActor;
			REDHOOK::SET_TIME_SCALE(1.0f); // Restore normal time scale
			Camera gameCam = CAM::GET_GAME_CAMERA();
			if (gameCam)
			{
				CAMERA::SET_CAMERA_DIRECTION(gameCam, { s_OrigCamDir.x , s_OrigCamDir.y}, 0.0f, true);
			}
		}
	}
}
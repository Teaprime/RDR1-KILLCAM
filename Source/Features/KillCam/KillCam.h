#pragma once
#include <unordered_set>

class KillCam
{
	public:
		static void Update();
		static void OnActorKilled(int actorId);
		static inline float s_TriggerProbability = 0.30f;
		static inline float s_KillCamDuration = 0.4f;
		static inline bool s_DoOnFinalEnemy = true;

	private:
		static void UpdateActorKillDetection();

		static inline bool s_KillCamEnabled = false;
		static inline float s_KillCamTimer = 0.0f;

		static inline int s_TargetActor = -1;
		static inline Vector3 s_OrigCamDir = { 0.0f, 0.0f, 0.0f };
		static inline float s_OrigHeading = 0.0f;

		static inline std::unordered_set<int> s_handledDead;

		// toggle between classic and experimental killcam
		static inline bool s_UseExperimental = true;
};
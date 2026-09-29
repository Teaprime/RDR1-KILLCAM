#include <pch.h>
#include "Headers/inicpp.h"


void Application::Initialize(HMODULE _Module)
{
	// This is now longer needed since RedHook v0.8 (If you still want to use it, uncomment this line)
	// InputsManager::Register();

	ScriptRegister(_Module, []
	{
		ini::IniFile config;
		config.load("./KillCam.ini");
		KillCam::s_TriggerProbability = config["Settings"]["Chance"].as<float>();
		KillCam::s_DoOnFinalEnemy = config["Settings"]["AlwaysTriggerWhenNoEnemiesLeft"].as<bool>();
		CameraExperiments::s_Timescale = config["Settings"]["Timescale"].as<float>();
		CameraExperiments::s_CutBackTimescale = config["Settings"]["TwoShotTimescale"].as<float>();
		CameraExperiments::s_CutBackChance = config["Settings"]["TwoShotChance"].as<float>();
		CameraExperiments::s_Duration = config["Settings"]["Duration"].as<float>();
		CameraExperiments::s_FirstshotDuration = config["Settings"]["FirstShotDuration"].as<float>();
		CameraExperiments::s_doFollowCam = config["Settings"]["AlwaysTrack"].as<bool>();

		while (true)
		{
			KillCam::Update();
			CameraExperiments::Update();

			ScriptWait(0);
		}
	});
}



void Application::Shutdown(HMODULE _Module)
{
	ScriptUnregister(_Module);

	// This is now longer needed since RedHook v0.8 (If you still want to use it, uncomment this line)
	// InputsManager::Unregister();
}
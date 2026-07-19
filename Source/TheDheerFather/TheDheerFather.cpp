#include "TheDheerFather.h"
#include "Modules/ModuleManager.h"
#include "AbilitySystemGlobals.h"

/**
 * Custom primary game module so we can call InitGlobalData() — required by GAS for
 * target data / gameplay cues. Without it, certain ability features assert at runtime.
 */
class FTheDheerFatherModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		UAbilitySystemGlobals::Get().InitGlobalData();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FTheDheerFatherModule, TheDheerFather, "TheDheerFather");

#include "MeshWeightRemapToolsetModule.h"

#include "MeshWeightRemapToolset.h"
#include "ToolsetRegistry/UToolsetRegistry.h"

DEFINE_LOG_CATEGORY(LogMeshWeightRemapToolset);

void FMeshWeightRemapToolsetModule::StartupModule()
{
	if (!UToolsetRegistry::IsAvailable())
	{
		return;
	}

	if (UToolsetRegistry::IsToolsetClassRegistered(UMeshWeightRemapToolset::StaticClass()))
	{
		bRegistered = true;
		return;
	}

	// Skills auto-discover. Toolsets do not - forget this line and the whole toolset is invisible,
	// with no warning anywhere, which reads exactly like the plugin being disabled.
	UToolsetRegistry::RegisterToolsetClass(UMeshWeightRemapToolset::StaticClass());
	bRegistered = UToolsetRegistry::IsToolsetClassRegistered(UMeshWeightRemapToolset::StaticClass());

	UE_LOG(LogMeshWeightRemapToolset, Log, TEXT("Mesh Weight Remap toolset %s."),
		bRegistered ? TEXT("registered") : TEXT("failed to register"));
}

void FMeshWeightRemapToolsetModule::ShutdownModule()
{
	if (bRegistered && UToolsetRegistry::IsAvailable())
	{
		UToolsetRegistry::UnregisterToolsetClass(UMeshWeightRemapToolset::StaticClass());
		bRegistered = false;
	}
}

IMPLEMENT_MODULE(FMeshWeightRemapToolsetModule, MeshWeightRemapToolset)

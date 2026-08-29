#include "MeshWeightRemapToolset.h"

#include "Editor.h"
#include "Kismet/KismetSystemLibrary.h"

UMeshWeightRemapSubsystem* UMeshWeightRemapToolset::GetSubsystemChecked()
{
	UMeshWeightRemapSubsystem* Subsystem =
		GEditor ? GEditor->GetEditorSubsystem<UMeshWeightRemapSubsystem>() : nullptr;

	if (!Subsystem)
	{
		UKismetSystemLibrary::RaiseScriptError(
			TEXT("Mesh Weight Remap is not available. The plugin is editor-only - check it is enabled."));
	}

	return Subsystem;
}

FMeshWeightRemapLeaderInfo UMeshWeightRemapToolset::InspectLeader(const FString& LeaderMeshPath,
	int32 LODIndex, EMeshWeightRemapDrivenSet DrivenSet)
{
	UMeshWeightRemapSubsystem* Subsystem = GetSubsystemChecked();
	if (!Subsystem)
	{
		return FMeshWeightRemapLeaderInfo();
	}

	FMeshWeightRemapLeaderInfo Info = Subsystem->InspectLeader(LeaderMeshPath, LODIndex, DrivenSet);
	if (Info.RequiredBones == 0)
	{
		UKismetSystemLibrary::RaiseScriptError(FString::Printf(
			TEXT("Could not measure '%s' at LOD %d: %s"),
			*LeaderMeshPath, LODIndex, *FString::Join(Info.Problems, TEXT(" "))));
	}

	return Info;
}

TArray<FMeshWeightRemapBoneWeight> UMeshWeightRemapToolset::GetWeightedBones(const FString& MeshPath,
	int32 LODIndex)
{
	UMeshWeightRemapSubsystem* Subsystem = GetSubsystemChecked();
	if (!Subsystem)
	{
		return TArray<FMeshWeightRemapBoneWeight>();
	}

	TArray<FMeshWeightRemapBoneWeight> Weights = Subsystem->GetWeightedBones(MeshPath, LODIndex);
	if (Weights.IsEmpty())
	{
		UKismetSystemLibrary::RaiseScriptError(FString::Printf(
			TEXT("'%s' has no skin weights at LOD %d. Check the path names a skeletal mesh and that the LOD ")
			TEXT("exists - call Inspect Leader on the same asset to see how many LODs it has."),
			*MeshPath, LODIndex));
	}

	return Weights;
}

FMeshWeightRemapReport UMeshWeightRemapToolset::RemapToLeader(const FString& TargetMeshPath,
	const FString& LeaderMeshPath, EMeshWeightRemapDrivenSet DrivenSet, int32 LODIndex, bool bDryRun)
{
	UMeshWeightRemapSubsystem* Subsystem = GetSubsystemChecked();
	if (!Subsystem)
	{
		return FMeshWeightRemapReport();
	}

	FMeshWeightRemapReport Report =
		Subsystem->RemapToLeader(TargetMeshPath, LeaderMeshPath, DrivenSet, LODIndex, bDryRun);

	if (!Report.bSucceeded)
	{
		UKismetSystemLibrary::RaiseScriptError(FString::Printf(
			TEXT("Remapping '%s' onto '%s' failed: %s Call Inspect Leader on '%s' to see what it drives, and ")
			TEXT("Get Weighted Bones on '%s' to see what needs driving."),
			*TargetMeshPath, *LeaderMeshPath,
			Report.Problems.IsEmpty()
				? TEXT("no reason was recorded.")
				: *FString::Join(Report.Problems, TEXT(" ")),
			*LeaderMeshPath, *TargetMeshPath));
	}

	return Report;
}

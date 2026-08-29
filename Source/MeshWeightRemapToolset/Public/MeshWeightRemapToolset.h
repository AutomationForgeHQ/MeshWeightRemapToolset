// MeshWeightRemap as MCP tools. Adapter only.

#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/ToolsetDefinition.h"
#include "MeshWeightRemapSubsystem.h"
#include "MeshWeightRemapToolset.generated.h"

/**
 * Makes skeletal meshes safe to leader-pose off one another.
 *
 * A follower reads its pose out of the leader's transform array, and the leader only ever writes the
 * bones it evaluates. A follower vertex weighted to any other bone therefore holds the reference pose
 * forever, which on a garment looks like a cuff pinned at the wrist while the arm swings. The weights
 * are not wrong and the skeletons are not mismatched; the leader simply never computes those bones.
 *
 * These tools find the bones a target has weight on and a leader will not drive, and move each one's
 * influence to the nearest ancestor it does drive - leaving every driven bone's weight untouched,
 * which is what makes this cheaper and better than re-transferring weights from the leader.
 */
UCLASS()
class MESHWEIGHTREMAPTOOLSET_API UMeshWeightRemapToolset : public UToolsetDefinition
{
	GENERATED_BODY()

public:

	virtual FString GetToolsetVersion() const override { return TEXT("0.1"); }

	/**
	 * Measure what a leader mesh actually drives. Writes nothing.
	 *
	 * Call this before remapping anything. It reports all three candidate definitions of "driven" side
	 * by side so the right one can be chosen on evidence. It also reports the reference skeleton's bone
	 * count, which is shared between every mesh built on that skeleton and therefore proves nothing -
	 * it is there so it is not mistaken for the answer.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshWeightRemap|Measure")
	static FMeshWeightRemapLeaderInfo InspectLeader(const FString& LeaderMeshPath, int32 LODIndex = 0,
		EMeshWeightRemapDrivenSet DrivenSet = EMeshWeightRemapDrivenSet::RequiredBones);

	/**
	 * Per-bone skin weight on a mesh: which bones carry influence, over how many vertices, totalling
	 * how much. Writes nothing.
	 *
	 * The count of returned entries is the figure to compare against a mesh already known to work. Two
	 * meshes that differ only in the bones they are bound to will differ here and nowhere else.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshWeightRemap|Measure")
	static TArray<FMeshWeightRemapBoneWeight> GetWeightedBones(const FString& MeshPath, int32 LODIndex = 0);

	/**
	 * Move a mesh's weights off everything the leader will not drive.
	 *
	 * Edits the target in place, leaving its package dirty and unsaved; there is no undo, because bone
	 * removal rebuilds the LOD model. Run it once with bDryRun true and read the report before running
	 * it for real - the dry report is complete apart from the after-numbers.
	 *
	 * Bone removal is a persistent setting on the mesh and it accumulates, so remapping the same mesh
	 * against a second leader adds to the first result rather than replacing it. The report names any
	 * bones a previous run already took off.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshWeightRemap|Edit")
	static FMeshWeightRemapReport RemapToLeader(const FString& TargetMeshPath, const FString& LeaderMeshPath,
		EMeshWeightRemapDrivenSet DrivenSet = EMeshWeightRemapDrivenSet::RequiredBones,
		int32 LODIndex = 0, bool bDryRun = false);

private:

	static UMeshWeightRemapSubsystem* GetSubsystemChecked();
};

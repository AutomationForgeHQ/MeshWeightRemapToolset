// What an agent needs to know about leader posing that the tool signatures cannot say.

#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/AgentSkill.h"
#include "MeshWeightRemapSkill.generated.h"

/**
 * Why a leader-posed mesh pins, and how to prove which bones are to blame.
 *
 * Deliberately free of tool and property names, which rot. What belongs here is the mechanism and the
 * measurements that settle an argument about it.
 */
UCLASS()
class MESHWEIGHTREMAPTOOLSET_API UMeshWeightRemapSkill : public UAgentSkill
{
	GENERATED_BODY()

public:

	UMeshWeightRemapSkill()
	{
		Description = TEXT(
			"Diagnose and fix a skeletal mesh that goes stiff when it is leader-posed off another one - a "
			"garment whose cuff or collar stays put while the body moves.");

		Instructions = TEXT(
			"Leader posing does not copy a pose. The follower reads bone transforms straight out of the "
			"leader's array, and the leader only ever writes the bones it evaluates. Every other entry in "
			"that array still holds the reference pose it was initialised with. So a follower vertex "
			"weighted to a bone the leader does not evaluate is not merely unanimated - it is held at the "
			"reference pose while everything around it moves, which reads as a pinned cuff, a stuck "
			"collar, or a stretched web of triangles between the two.\n"
			"\n"
			"The set the leader evaluates is a property of one LOD of one mesh. It is not a property of "
			"the skeleton, and this is where most of the time gets lost: meshes built on a shared "
			"skeleton all report the same reference bone list, identically, whether they are a whole body "
			"or a single glove. That number distinguishes nothing and should never be quoted as evidence. "
			"Measure the leader's per-LOD evaluated set instead; it is the only number that decides "
			"anything, and there is a tool that reports it precisely so nobody has to guess.\n"
			"\n"
			"The fix is to move weight, not to rebuild it. Take every bone the follower has weight on that "
			"the leader will not drive, and push its influence up to the nearest ancestor the leader does "
			"drive. Weights on driven bones are then bit-for-bit what they were, which matters more than "
			"it sounds: the obvious alternative - re-transferring the follower's weights from the leader "
			"mesh - also stops the pinning, and quietly throws away all the resolution the original "
			"binding had wherever the leader has no geometry. On a garment that is the neck and the "
			"shoulders, and the loss does not show up until somebody turns their head.\n"
			"\n"
			"How to work, in order. Measure the leader first and write down what it drives. If some other "
			"mesh in the project already leader-poses off that same leader correctly, measure its weighted "
			"bones too - that is a free oracle, and matching its count exactly is a much stronger result "
			"than a fix that merely looks better in the viewport. Then run the remap as a dry pass, which "
			"reports every bone it would empty and where each one's weight would go, and read that list "
			"before writing anything. Only then run it for real.\n"
			"\n"
			"Two things to check in every report. Total weight must be identical before and after - weight "
			"is moved, never created or destroyed, so a total that has shifted means the bone count above "
			"it cannot be trusted and the asset should be discarded unsaved. And the bones that were "
			"emptied must genuinely carry nothing afterwards; a bone that survives the pass is a bone the "
			"engine refused to remove, and the reason will be in the log.\n"
			"\n"
			"Finally, treat this as irreversible. Removing a bone rebuilds the LOD, so there is no undo, "
			"and the removal is stored on the mesh and accumulates - remapping the same mesh against a "
			"second leader adds to what the first run took off rather than replacing it. Remapping "
			"against the wrong leader is therefore not a mistake you can walk back by running it again "
			"correctly. The package is left dirty and unsaved, which is the one escape hatch: if the "
			"numbers are wrong, do not save.");
	}
};

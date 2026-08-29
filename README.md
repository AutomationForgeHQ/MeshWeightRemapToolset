# Mesh Weight Remap Toolset

`MeshWeightRemap` as native Model Context Protocol tools, so an agent can measure and fix a whole
folder of garments in one pass instead of one right-click at a time.

**Adapter only.** It holds no logic — every tool forwards to `UMeshWeightRemapSubsystem` and turns a
failure into a script error. The reason it is a separate plugin is that `ToolsetRegistry` and
`ModelContextProtocol` are experimental: folding the adapter into the capability would mean the
capability will not load anywhere those are switched off.

For what the tool actually does and why, read [the capability's README](../MeshWeightRemap/README.md)
first. This document only covers the MCP surface.

---

## Enabling it

Both plugins, plus `ToolsetRegistry` and `ModelContextProtocol`. Registration is explicit and silent
when it fails, so check the log on startup:

```
LogToolsetRegistry: Display: Registering Toolset MeshWeightRemapToolset.MeshWeightRemapToolset
LogMeshWeightRemapToolset: Mesh Weight Remap toolset registered.
```

No line means the toolset is invisible to every client, which reads exactly like the plugin being
disabled.

## The tools

Toolset name: `MeshWeightRemapToolset.MeshWeightRemapToolset`.

### `InspectLeader(leaderMeshPath, lODIndex = 0, drivenSet = RequiredBones)`

What a leader mesh actually drives. Writes nothing. **Call this first.**

Reports all three candidate definitions of driven side by side — `requiredBones`, `activeBones`,
`extraPhysicsAssetBones` — so the right one is chosen on evidence rather than assumed. Also returns
`referenceSkeletonBones`, which is shared by every mesh built on that skeleton and therefore proves
nothing; it is there so it is not mistaken for the answer.

`bFromRenderData` says whether the numbers came from cooked render data (what the animation system
actually reads) or from the imported model.

### `GetWeightedBones(meshPath, lODIndex = 0)`

Per-bone influence: which bones carry weight, over how many vertices, totalling how much. Writes
nothing. Sorted heaviest first.

The number of entries is the figure to compare against a mesh already known to work, and it is the same
quantity a per-bone audit of an exported FBX reports.

### `RemapToLeader(targetMeshPath, leaderMeshPath, drivenSet = RequiredBones, lODIndex = 0, bDryRun = false)`

Moves the target's weights off everything the leader will not drive.

Edits in place, leaves the package dirty and unsaved, and **there is no undo** — bone removal rebuilds
the LOD model. Run it with `bDryRun` true first; the dry report is complete apart from the
after-numbers, and it names every bone it would empty and where each one's weight would go.

## The order that works

1. `InspectLeader` on the leader.
2. If some other mesh already leader-poses off that leader correctly, `RemapToLeader` it with
   `bDryRun`. **A working mesh must come back with zero orphans.** That is the strongest available
   check that the chosen definition of driven is the right one, and it costs one call.
3. `RemapToLeader` with `bDryRun` on the real target. Read the list.
4. `RemapToLeader` for real.
5. Save, if and only if `totalWeightAfter` equals `totalWeightBefore` and `problems` is empty.

Step 2 is the one people skip. A target bone count taken from a good asset by diffing it against a bad
one will include every difference between the two, not only the defect — running the tool on the good
asset is what separates those.

## Reading the report

| field | what it is for |
|---|---|
| `weightedBonesBefore` / `After` | the figure an external weight audit reports |
| `totalWeightBefore` / `After` | **must match.** Weight is moved, never created or destroyed |
| `orphanBones` / `moved` | what was emptied and where each one's weight went |
| `previouslyRemoved` | bones an earlier run already took off — removal accumulates |
| `targetRequiredBonesBefore` / `After` | the mesh's own per-LOD bone count, a different quantity |
| `problems` | empty on a clean run |

A `totalWeight` that has shifted by more than requantisation invalidates the bone count above it —
discard the asset without saving.

## The agent skill

`UMeshWeightRemapSkill` ships with the plugin and is discovered automatically — no registration. It
carries the mechanism and the failure modes, which is the part no signature can express: why leader
posing pins a vertex, why the reference skeleton count is worthless, and why re-transferring weights
from the leader is the wrong fix.

## Deliberately absent

There is **no batch tool**. Remapping is irreversible and accumulates, so the fan-out belongs in the
caller's loop where each report can be read, not hidden inside one call that reports a total. Nobody
should "fix" this by adding one.

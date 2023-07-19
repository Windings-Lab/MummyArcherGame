// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkeletalMeshMerge.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshMergeFunctionLibrary.generated.h"

/**
* Struct containing all parameters used to perform a Skeletal Mesh merge.
*/
USTRUCT(BlueprintType)
struct MUMMYARCHERGAME_API FSkeletalMeshMergeParams
{
    GENERATED_BODY()
        FSkeletalMeshMergeParams()
    {
        StripTopLODS = 0;
        bNeedsCpuAccess = false;
        bSkeletonBefore = false;
        Skeleton = nullptr;
    }
    // An optional array to map sections from the source meshes to merged section entries
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray <FSkelMeshMergeSectionMapping > SectionMappings;
    
    // An optional array to transform the UVs in each mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FSkelMeshMergeUVTransformMapping UVTransformMappings;
    
    // The list of skeletal meshes to merge.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray < USkeletalMesh* > MeshesToMerge;
    
    // The number of high LODs to remove from input meshes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 StripTopLODS;
    
    // Whether or not the resulting mesh needs to be accessed by the CPU for any reason (e.g. for spawning particle effects).
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        uint32 bNeedsCpuAccess : 1;
    
    // Update skeleton before merge. Otherwise, update after.
    // Skeleton must also be provided.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        uint32 bSkeletonBefore : 1;
    
    // Skeleton that will be used for the merged mesh.
    // Leave empty if the generated skeleton is OK.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        USkeleton* Skeleton;
};
/**
*
*/
UCLASS()
class MUMMYARCHERGAME_API UMeshMergeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* Merges the given meshes into a single mesh.
	* @return The merged mesh (will be invalid if the merge failed).
	*/
	UFUNCTION(BlueprintCallable, Category = "Mesh Merge", meta = (UnsafeDuringActorConstruction = "true"))
	static void MergeAndSaveMeshes(const FSkeletalMeshMergeParams& Params);
};

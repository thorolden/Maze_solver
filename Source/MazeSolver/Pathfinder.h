#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pathfinder.generated.h"

USTRUCT()
struct FPathNode
{
    GENERATED_BODY()

    int32 X;
    int32 Y;
    int32 G;
    int32 H;
    int32 F;
    int32 ParentX;
    int32 ParentY;

    FPathNode() : X(0), Y(0), G(0), H(0), F(0), ParentX(-1), ParentY(-1) {}
    FPathNode(int32 InX, int32 InY, int32 InG, int32 InH, int32 InParentX, int32 InParentY)
        : X(InX), Y(InY), G(InG), H(InH), F(InG + InH), ParentX(InParentX), ParentY(InParentY) {}
};

UCLASS()
class MAZESOLVER_API APathfinder : public AActor
{
    GENERATED_BODY()
	
public:	
	APathfinder();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector2D StartCell = FVector2D(1, 0); // Вход

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector2D EndCell = FVector2D(0, 0); // Выход (установим в BeginPlay)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MoveSpeed = 200.f;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void FindAndFollowPath();

private:
    TArray<FVector2D> Path;
    int32 PathIndex = 0;
    FVector TargetLocation;
    bool bFollowingPath = false;

    const TArray<TArray<int32>>* MazePtr = nullptr;

    void MoveAlongPath(float DeltaTime);
    bool FindPathAStar(const TArray<TArray<int32>>& Maze, FVector2D Start, FVector2D End, TArray<FVector2D>& OutPath);

    // Add the missing declaration for VisualizePath
    void VisualizePath();
};

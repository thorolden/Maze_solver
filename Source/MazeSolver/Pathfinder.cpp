// Fill out your copyright notice in the Description page of Project Settings.
#include "Pathfinder.h"
#include "MazeGenerator.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" // For path visualization

// Sets default values
APathfinder::APathfinder()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APathfinder::BeginPlay()
{
	Super::BeginPlay();

	// Найти MazeGenerator на уровне
	for (TActorIterator<AMazeGenerator> It(GetWorld()); It; ++It)
	{
		MazePtr = &It->GetMazeData();
		break;
	}

	if (MazePtr && MazePtr->Num() > 0)
	{
		// Поиск выхода (последний проход в нижней строке)
		int32 W = (*MazePtr)[0].Num();
		int32 H = MazePtr->Num();
		for (int32 x = 0; x < W; ++x)
		{
			if ((*MazePtr)[H - 1][x] == 0)
			{
				EndCell = FVector2D(x, H - 1);
				break;
			}
		}
		FindAndFollowPath();
	}
}

// Called every frame
void APathfinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bFollowingPath)
	{
		MoveAlongPath(DeltaTime);
	}
}

void APathfinder::FindAndFollowPath()
{
	if (!MazePtr) return;
	if (FindPathAStar(*MazePtr, StartCell, EndCell, Path))
	{
		PathIndex = 0;
		if (Path.Num() > 1)
		{
			TargetLocation = FVector(static_cast<int32>(Path[0].X) * 100.f, static_cast<int32>(Path[0].Y) * 100.f, GetActorLocation().Z);
			SetActorLocation(TargetLocation);
			PathIndex = 1;
			bFollowingPath = true;
		}
		VisualizePath();
	}
}

void APathfinder::VisualizePath()
{
	if (Path.Num() < 2) return;

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 i = 1; i < Path.Num(); ++i)
	{
		FVector Start = FVector(static_cast<int32>(Path[i - 1].X) * 100.f, static_cast<int32>(Path[i - 1].Y) * 100.f, GetActorLocation().Z + 20.f);
		FVector End   = FVector(static_cast<int32>(Path[i].X) * 100.f, static_cast<int32>(Path[i].Y) * 100.f, GetActorLocation().Z + 20.f);
		DrawDebugLine(World, Start, End, FColor::Green, true, 30.0f, 0, 8.0f); // Green color
	}
}

void APathfinder::MoveAlongPath(float DeltaTime)
{
	if (PathIndex >= Path.Num())
	{
		bFollowingPath = false;
		return;
	}
	FVector NextLocation = FVector(static_cast<int32>(Path[PathIndex].X) * 100.f, static_cast<int32>(Path[PathIndex].Y) * 100.f, GetActorLocation().Z);
	FVector Current = GetActorLocation();
	FVector Direction = (NextLocation - Current).GetSafeNormal();
	float Distance = FVector::Dist(Current, NextLocation);

	if (Distance < 5.f)
	{
		PathIndex++;
	}
	else
	{
		SetActorLocation(Current + Direction * MoveSpeed * DeltaTime);
	}
}

bool APathfinder::FindPathAStar(const TArray<TArray<int32>>& Maze, FVector2D Start, FVector2D End, TArray<FVector2D>& OutPath)
{
	int32 W = Maze[0].Num();
	int32 H = Maze.Num();
	TArray<FPathNode> OpenList;
	TArray<FPathNode> ClosedList;

	auto Heuristic = [](int32 X1, int32 Y1, int32 X2, int32 Y2)
	{
		return FMath::Abs(X1 - X2) + FMath::Abs(Y1 - Y2);
	};

	OpenList.Add(FPathNode(Start.X, Start.Y, 0, Heuristic(Start.X, Start.Y, End.X, End.Y), -1, -1));

	while (OpenList.Num() > 0)
	{
		// Найти с минимальным F
		int32 MinIdx = 0;
		for (int32 i = 1; i < OpenList.Num(); ++i)
			if (OpenList[i].F < OpenList[MinIdx].F)
				MinIdx = i;

		FPathNode Current = OpenList[MinIdx];
		OpenList.RemoveAt(MinIdx);
		ClosedList.Add(Current);

		if (Current.X == End.X && Current.Y == End.Y)
		{
			// Восстановить путь
			TArray<FVector2D> RevPath;
			FPathNode* Node = &ClosedList.Last();
			while (Node->ParentX != -1 && Node->ParentY != -1)
			{
				RevPath.Add(FVector2D(Node->X, Node->Y));
				for (FPathNode& N : ClosedList)
				{
					if (N.X == Node->ParentX && N.Y == Node->ParentY)
					{
						Node = &N;
						break;
					}
				}
			}
			RevPath.Add(Start);
			Algo::Reverse(RevPath);
			OutPath = RevPath;
			return true;
		}

		// Проверить соседей
		const int32 DX[4] = { 0, 0, -1, 1 };
		const int32 DY[4] = { -1, 1, 0, 0 };
		for (int32 dir = 0; dir < 4; ++dir)
		{
			int32 NX = Current.X + DX[dir];
			int32 NY = Current.Y + DY[dir];
			if (NX < 0 || NX >= W || NY < 0 || NY >= H) continue;
			if (Maze[NY][NX] != 0) continue;
			bool bInClosed = false;
			for (const FPathNode& N : ClosedList)
				if (N.X == NX && N.Y == NY) { bInClosed = true; break; }
			if (bInClosed) continue;

			int32 G = Current.G + 1;
			int32 HScore = Heuristic(NX, NY, End.X, End.Y);
			bool bInOpen = false;
			for (FPathNode& N : OpenList)
			{
				if (N.X == NX && N.Y == NY)
				{
					if (G < N.G)
					{
						N.G = G;
						N.F = G + HScore;
						N.ParentX = Current.X;
						N.ParentY = Current.Y;
					}
					bInOpen = true;
					break;
				}
			}
			if (!bInOpen)
			{
				OpenList.Add(FPathNode(NX, NY, G, HScore, Current.X, Current.Y));
			}
		}
	}
	return false;
}


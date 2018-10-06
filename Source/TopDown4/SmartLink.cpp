// Fill out your copyright notice in the Description page of Project Settings.

#include "SmartLink.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavLinkCustomComponent.h"
#include "NavLinkRenderingComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "AI/NavigationSystemHelpers.h"
#include "VisualLogger/VisualLogger.h"
#include "NavigationOctree.h"
#include "ObjectEditorUtils.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "AI/NavigationSystemBase.h"


// Fill out your copyright notice in the Description page of Project Settings.


// Sets default values
ASmartLink::ASmartLink()
{
	// We don't need to Tick
	PrimaryActorTick.bCanEverTick = false;

	// Create our Components and setup default values
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	NavLink = CreateDefaultSubobject<UNavLinkCustomComponent>(TEXT("Nav Link"));
	NavLink->SetMoveReachedLink(this, &ASmartLink::NotifySmartLinkReached);
	//NavLink->SetupAttachment(Root);

	// Set a default target to be slightly outside our mesh
	Left = { -100.f, 0.f, 0.f };
	Right = { 100.f, 0.f, 0.f };

#if WITH_EDITOR
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (!IsRunningCommandlet() && (SpriteComponent != NULL))
	{
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture;
			FName ID_Decals;
			FText NAME_Decals;
			FConstructorStatics()
				: SpriteTexture(TEXT("/Engine/EditorResources/AI/S_NavLink"))
				, ID_Decals(TEXT("Navigation"))
				, NAME_Decals(NSLOCTEXT("SpriteCategory", "Navigation", "Navigation"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.SpriteTexture.Get();
		SpriteComponent->RelativeScale3D = FVector(0.5f, 0.5f, 0.5f);
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->bVisible = true;
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Decals;
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Decals;
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->SetAbsolute(false, false, true);
		SpriteComponent->bIsScreenSizeScaled = true;
	}
#endif
}

void ASmartLink::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateNavLinks();
}

void ASmartLink::UpdateNavLinks()
{
	FVector lastLeft, lastRight;
	ENavLinkDirection::Type lastDirection;

	NavLink->GetLinkData(lastLeft, lastRight, lastDirection);
	NavLink->SetLinkData(Left, Right, lastDirection);

	// Force rebuild of local NavMesh
	FNavigationSystem::UpdateComponentData(*NavLink);
}

FBox ASmartLink::GetComponentsBoundingBox(bool bNonColliding) const
{
	FBox LinksBB(FVector(0.f, 0.f, -10.f), FVector(0.f, 0.f, 10.f));
	LinksBB = LinksBB.TransformBy(RootComponent->GetComponentTransform());
	LinksBB += NavLink->GetStartPoint();
	LinksBB += NavLink->GetEndPoint();

	return LinksBB;
}

// Called when the game starts or when spawned
void ASmartLink::BeginPlay()
{

#if ENABLE_VISUAL_LOG
	/*
	UNavigationSystem* NavSys = UNavigationSystem::GetCurrent(GetWorld());
	if (NavSys)
	{
	REDIRECT_OBJECT_TO_VLOG(this, NavSys);
	}
	*/
#endif // ENABLE_VISUAL_LOG

	Super::BeginPlay();
}

// Called every frame
void ASmartLink::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASmartLink::NotifySmartLinkReached(UNavLinkCustomComponent* LinkComp, UObject* PathComp, const FVector& DestPoint)
{
	AActor* PathOwner = ((UPathFollowingComponent*)PathComp)->GetOwner();
	AController* ControllerOwner = Cast<AController>(PathOwner);
	APawn *pawn = nullptr;
	if (ControllerOwner)
	{
		pawn = ControllerOwner->GetPawn();
	}

	GEngine->AddOnScreenDebugMessage(0, 20.f, FColor::Red, FString::Printf(TEXT("SmartLink reached: x: %f, y: %f, z: %f"), DestPoint.X, DestPoint.Y, DestPoint.Z));
	ReceiveSmartLinkReached(pawn, DestPoint);
	ReceiveSmartLinkReachedWithReturn(pawn, DestPoint);
	OnSmartLinkReached.Broadcast(pawn, DestPoint);
}

void ASmartLink::ResumePathFollowing(AActor* Agent)
{
	if (Agent)
	{
		UPathFollowingComponent* PathComp = Agent->FindComponentByClass<UPathFollowingComponent>();
		if (PathComp == NULL)
		{
			APawn* PawnOwner = Cast<APawn>(Agent);
			if (PawnOwner && PawnOwner->GetController())
			{
				PathComp = PawnOwner->GetController()->FindComponentByClass<UPathFollowingComponent>();
			}
		}

		if (PathComp)
		{
			PathComp->FinishUsingCustomLink(NavLink);
		}
	}
}

bool ASmartLink::IsEnabled() const
{
	return NavLink->IsEnabled();
}

void ASmartLink::SetEnabled(bool bEnabled)
{
	NavLink->SetEnabled(bEnabled);
}

bool ASmartLink::HasMovingAgents() const
{
	return NavLink->HasMovingAgents();
}

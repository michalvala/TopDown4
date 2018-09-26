// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/Actor.h"
#include "EngineDefines.h"
#include "AI/Navigation/NavRelevantInterface.h"
#include "AI/Navigation/NavLinkDefinition.h"
#include "NavLinkHostInterface.h"

#include "SmartLink.generated.h"

class UBillboardComponent;
class UNavLinkCustomComponent;
class UNavLinkRenderingComponent;
class UPathFollowingComponent;
struct FNavigationRelevantData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSmartLinkReachedSignature, AActor*, MovingActor, const FVector&, DestinationPoint);

UCLASS(Blueprintable, autoCollapseCategories = (SmartLink, Actor), hideCategories = (Input))
class TOPDOWN4_API ASmartLink : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASmartLink();

	UPROPERTY(VisibleAnywhere)
		USceneComponent* Root;

	// This component's details are automatically set in the Construction Script
	UPROPERTY(VisibleAnywhere)
		UNavLinkCustomComponent* NavLink;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MakeEditWidget = ""))
		FVector Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MakeEditWidget = ""))
		FVector Right;


	//////////////////////////////////////////////////////////////////////////
	// Blueprint interface for smart links

	/** called when agent reaches smart link during path following, use ResumePathFollowing() to give control back */
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveSmartLinkReached(APawn* Pawn, const FVector& Destination);

	UFUNCTION(BlueprintImplementableEvent)
		bool ReceiveSmartLinkReachedWithReturn(APawn* Pawn, const FVector& Destination);

	/** resume normal path following */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
		void ResumePathFollowing(AActor* Agent);

	/** check if smart link is enabled */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
		bool IsEnabled() const;

	/** change state of smart link */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
		void SetEnabled(bool bEnabled);

	/** check if any agent is moving through smart link right now */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
		bool HasMovingAgents() const;

protected:
	UPROPERTY(BlueprintAssignable)
		FSmartLinkReachedSignature OnSmartLinkReached;

	void NotifySmartLinkReached(UNavLinkCustomComponent* LinkComp, UObject* PathComp, const FVector& DestPoint);


#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
		UBillboardComponent* SpriteComponent;
#endif // WITH_EDITORONLY_DATA


public:
	virtual FBox GetComponentsBoundingBox(bool bNonColliding = false) const override;

private:
	// Whenever we edit the Actor Transform or any properties via the Details panel, this will trigger.
	void OnConstruction(const FTransform& Transform) override;

	// Update the Nav Link Component to link from the Left to the Right
	void UpdateNavLinks();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};

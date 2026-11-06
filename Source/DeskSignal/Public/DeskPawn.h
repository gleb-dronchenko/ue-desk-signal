#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "DeskPawn.generated.h"

// A pawn that represents a desk
UCLASS()
class DESKSIGNAL_API ADeskPawn : public ADefaultPawn
{
    // generated body
	GENERATED_BODY()

    public:
        ADeskPawn();

        // poll the desk for the latest data
        UFUNCTION(BlueprintCallable)
        void Poll();

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        bool Forward = false;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        bool Back = false;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        bool Left = false;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        bool Right = false;

        // whether the desk has been updated since the last poll
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        bool Fresh = false;

        // the URL of the desk
        UPROPERTY(EditInstanceOnly)
        FString Url;

        // setup the player input component
        virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    private:
        // a struct to represent the body of the desk
        void CompletePoll(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

        TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ActiveRequest;

        // apply the body to the desk
        void Apply(FHttpRequestPtr Request, int32 Code, const FString& Body, bool bSucceeded);
};
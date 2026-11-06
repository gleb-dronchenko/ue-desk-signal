#include "DeskPawn.h"
#include "Components/InputComponent.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"

namespace
{

    // a struct to represent the body of the desk
    struct FDeskBody
    {
        bool Forward = false;
        bool Back = false;
        bool Left = false;
        bool Right = false;
        bool Fresh = false;
    };

    TOptional<FDeskBody> ParseBody(FStringView Body)
    {
        // parse the body as a JSON object
        TSharedPtr<FJsonObject> Object;

        // create a JSON reader from the body
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::CreateFromView(Body);

        // deserialize the body into a JSON object
        if (!FJsonSerializer::Deserialize(Reader, Object) || !Object.IsValid())
        {
            return {};
        }

        // parse the body into a FDeskBody object
        FDeskBody Parsed;

        // check if the body has the forward field
        if (!Object->HasTypedField<EJson::Boolean>(TEXT("forward"))
            || !Object->TryGetBoolField(TEXT("forward"), Parsed.Forward)
            || !Object->HasTypedField<EJson::Boolean>(TEXT("back"))
            || !Object->TryGetBoolField(TEXT("back"), Parsed.Back)
            || !Object->HasTypedField<EJson::Boolean>(TEXT("left"))
            || !Object->TryGetBoolField(TEXT("left"), Parsed.Left)
            || !Object->HasTypedField<EJson::Boolean>(TEXT("right"))
            || !Object->TryGetBoolField(TEXT("right"), Parsed.Right)
            || !Object->HasTypedField<EJson::Boolean>(TEXT("fresh"))
            || !Object->TryGetBoolField(TEXT("fresh"), Parsed.Fresh))
        {
            return {};
        }

        return Parsed;
    }

}

ADeskPawn::ADeskPawn()
{
}

void ADeskPawn::Poll()
{
    // if there is an active request, return
    if (ActiveRequest.IsValid())
    {
        return;
    }

    // if the URL is empty, set the fresh flag to false and return
    if (Url.IsEmpty())
    {
        Fresh = false;
        return;
    }

    // create a new HTTP request
    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb(TEXT("GET"));
    Request->SetURL(Url);
    Request->SetTimeout(1.f);
    Request->OnProcessRequestComplete().BindUObject(this, &ADeskPawn::CompletePoll);
    ActiveRequest = Request;

    // process the request
    if (!Request->ProcessRequest())
    {
        Request->OnProcessRequestComplete().Unbind();
        ActiveRequest.Reset();
        Fresh = false;
    }
}

void ADeskPawn::CompletePoll(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
    const int32 Code = Response.IsValid() ? Response->GetResponseCode() : 0;
    const FString Body = Response.IsValid() ? Response->GetContentAsString() : FString();
    const TWeakObjectPtr<ADeskPawn> WeakThis(this);

    AsyncTask(ENamedThreads::GameThread, [WeakThis, Request, Code, Body, bSucceeded]()
    {
        if (ADeskPawn* Pawn = WeakThis.Get())
        {
            Pawn->Apply(Request, Code, Body, bSucceeded);
        }
    });
}

void ADeskPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // add the default movement bindings
    bAddDefaultMovementBindings = true;

    // call the parent class to add the default movement bindings
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // remove the default movement bindings
    PlayerInputComponent->RemoveAxisBinding(TEXT("DefaultPawn_MoveForward"));
    PlayerInputComponent->RemoveAxisBinding(TEXT("DefaultPawn_MoveRight"));
    PlayerInputComponent->RemoveAxisBinding(TEXT("DefaultPawn_MoveUp"));
}

void ADeskPawn::Apply(FHttpRequestPtr Request, int32 Code, const FString& Body, bool bSucceeded)
{
    if (ActiveRequest != Request)
    {
        return;
    }

    ActiveRequest.Reset();

    const TOptional<FDeskBody> Parsed = ParseBody(Body);
    if (bSucceeded && Code == 200 && Parsed.IsSet())
    {
        Forward = Parsed->Forward;
        Back = Parsed->Back;
        Left = Parsed->Left;
        Right = Parsed->Right;
        Fresh = Parsed->Fresh;
        return;
    }

    Fresh = false;
}

void ADeskPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ActiveRequest.IsValid())
    {
        ActiveRequest->OnProcessRequestComplete().Unbind();
        ActiveRequest->CancelRequest();
        ActiveRequest.Reset();
    }

    Super::EndPlay(EndPlayReason);
}

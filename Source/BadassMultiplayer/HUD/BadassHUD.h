#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BadassHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	float CrosshairsSpread;
	FLinearColor CrosshairColor;
};

/**
 * 
 */

class UCharacterOverlay;

UCLASS()
class BADASSMULTIPLAYER_API ABadassHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UCharacterOverlay* CharacterOverlay;

protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();

private:
	FHUDPackage HUDPackage;
	void DrawCrosshairTexture(UTexture2D* Tex, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	// Allows us to control the spread from here and the logic will just control the direction and base amount
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
	
};

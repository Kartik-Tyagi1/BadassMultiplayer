#include "BadassHUD.h"

void ABadassHUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D VeiwportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairsSpread;

		if (HUDPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshairTexture(HUDPackage.CrosshairCenter, VeiwportCenter, Spread);
		}
		if (HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshairTexture(HUDPackage.CrosshairTop, VeiwportCenter, Spread);
		}
		if (HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshairTexture(HUDPackage.CrosshairBottom, VeiwportCenter, Spread);
		}
		if (HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshairTexture(HUDPackage.CrosshairLeft, VeiwportCenter, Spread);
		}
		if (HUDPackage.CrosshairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshairTexture(HUDPackage.CrosshairRight, VeiwportCenter, Spread);
		}
	}
}

void ABadassHUD::DrawCrosshairTexture(UTexture2D* Tex, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Tex->GetSizeX();
	const float TextureHeight = Tex->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2) + Spread.Y
	);

	DrawTexture(
		Tex,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White
	);
}

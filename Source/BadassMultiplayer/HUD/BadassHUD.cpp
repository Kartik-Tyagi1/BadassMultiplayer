#include "BadassHUD.h"

void ABadassHUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D VeiwportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

		if (HUDPackage.CrosshairCenter)
		{
			DrawCrosshairTexture(HUDPackage.CrosshairCenter, VeiwportCenter);
		}
		if (HUDPackage.CrosshairTop)
		{
			DrawCrosshairTexture(HUDPackage.CrosshairTop, VeiwportCenter);
		}
		if (HUDPackage.CrosshairBottom)
		{
			DrawCrosshairTexture(HUDPackage.CrosshairBottom, VeiwportCenter);
		}
		if (HUDPackage.CrosshairLeft)
		{
			DrawCrosshairTexture(HUDPackage.CrosshairLeft, VeiwportCenter);
		}
		if (HUDPackage.CrosshairRight)
		{
			DrawCrosshairTexture(HUDPackage.CrosshairRight, VeiwportCenter);
		}
	}
}

void ABadassHUD::DrawCrosshairTexture(UTexture2D* Tex, FVector2D ViewportCenter)
{
	const float TextureWidth = Tex->GetSizeX();
	const float TextureHeight = Tex->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2 ,
		ViewportCenter.Y - TextureHeight / 2
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

#pragma once

UENUM(BlueprintType)
enum class ETurningState : uint8
{
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_Still UMETA(DisplayName = "Still"),

	ETIP_MAX UMETA(DisplayName = "DefaultMAX")

};
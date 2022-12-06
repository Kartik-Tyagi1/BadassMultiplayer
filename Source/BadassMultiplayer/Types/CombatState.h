#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied	UMETA(DisplayName = "Unoccupied"), // NOT UNEQUIPPED -> This just means the character is not doing anything and can go into other states
	ECS_Reloading	UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
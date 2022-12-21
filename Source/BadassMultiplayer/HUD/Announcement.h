#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

/**
 * 
 */

class UTextBlock;

UCLASS()
class BADASSMULTIPLAYER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* WarmupTime;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* AnnouncemetText;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* InfoText;
	
};

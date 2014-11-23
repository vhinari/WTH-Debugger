
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLEVEHICLE_API APickUp : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = PickUp)
	bool bIsActivate;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = PickUp)
	INT16 iCountFlags;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = PickUp)
	TSubobjectPtr<USphereComponent> BaseCollision;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = PickUp)
	TSubobjectPtr<UStaticMeshComponent> PickupMesh;

	UFUNCTION(BlueprintNativeEvent)
	void PickedUp();
	
};

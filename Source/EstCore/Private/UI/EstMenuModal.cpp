// Estranged is a trade mark of Alan Edwardes.


#include "UI/EstMenuModal.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstMenuModal)

void UEstMenuModal::OnBack_Implementation()
{
	OnExit.Broadcast();
	Super::OnBack_Implementation();
}

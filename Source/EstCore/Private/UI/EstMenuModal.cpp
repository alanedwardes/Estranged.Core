// Estranged is a trade mark of Alan Edwardes.


#include "UI/EstMenuModal.h"

void UEstMenuModal::OnBack_Implementation()
{
	OnExit.Broadcast();
	Super::OnBack_Implementation();
}

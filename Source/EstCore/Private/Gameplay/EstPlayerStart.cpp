// Estranged is a trade mark of Alan Edwardes.


#include "Gameplay/EstLevelWaypoint.h"
#include "Gameplay/EstPlayerStart.h"

#if WITH_EDITOR
void AEstPlayerStart::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Waypoint != nullptr)
	{
		UEstLevelWaypoint* ClassDefaultObject = Waypoint->GetDefaultObject<UEstLevelWaypoint>();
		PlayerStartTag = ClassDefaultObject->Waypoint;
	}
}
#endif
#pragma once

#define TAG_DEAD "DEAD"
#define TAG_NOIMPACTS "NOIMPACTS"
#define TAG_NOSAVERESTORE "NOSAVERESTORE"

#define DEBUG_PERSIST_TIME 5.f
#define DEBUG_COLOR FColor::Green
#define WARNING_PERSIST_TIME 30.f
#define WARNING_COLOR FColor::Yellow

#define CHANNEL_PLAYER_IN_FRONT ECollisionChannel::ECC_GameTraceChannel6
#define CHANNEL_PLAYER_BLUR_FOCUS ECollisionChannel::ECC_GameTraceChannel7
#define CHANNEL_PLAYER_INTERACT ECollisionChannel::ECC_GameTraceChannel8
#define CHANNEL_PLAYER_WATER_CHECK ECollisionChannel::ECC_GameTraceChannel9
#define CHANNEL_FOOTSTEPS ECollisionChannel::ECC_GameTraceChannel14

#define PROFILE_TRIGGER "Trigger"
#define PROFILE_PLAYER "Player"
#define PROFILE_BULLET "Bullet"
#define PROFILE_DEBRIS "Debris"

#define SURFACE_TYPE_GLASS EPhysicalSurface::SurfaceType1
#define SURFACE_TYPE_FOLIAGE EPhysicalSurface::SurfaceType2
#define SURFACE_TYPE_METAL_MESH EPhysicalSurface::SurfaceType3
#define SURFACE_TYPE_WATER EPhysicalSurface::SurfaceType4
#define SURFACE_TYPE_FLESH EPhysicalSurface::SurfaceType5

#define WEAPON_SOCKET "WeaponSocket"
#define BARREL_SOCKET "BarrelSocket"
#define HOLD_SOCKET "HoldSocket"
#define HIGHLIGHT_MATERIAL_PARAMETER "Highlight"

#define BLACKBOARD_FOCUS_ACTOR "FocusActor"
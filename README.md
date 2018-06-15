# Estranged.Core
Source code for the core gameplay components of Estranged: Act II.

## Key Concepts
* EstBaseCharacter - root of all characters, inherits from ACharacter
   * EstPlayer - player character, includes interaction and picking up mechanics, and tracking of resources (energy, power)
* EstBaseWeapon - root of all weapons, inherits from AActor
   * EstFirearmWeapon - a weapon that can fire projectiles
* EstCharacterMovementComponent - provides additional movement like sprinting, responsible for scheduling footstep sounds
* EstFirearmAmunition - component providing tracking of bullets / clips for a specific type of amunition
* EstPlayerController - controller providing main menu visibility management
* EstPlayerHUD - provides heads up display support for the player (aware of weapons, amunition etc)

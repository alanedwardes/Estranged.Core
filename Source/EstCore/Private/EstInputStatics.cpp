#include "EstInputStatics.h"

THIRD_PARTY_INCLUDES_START
	#if PLATFORM_WINDOWS
		#include <SDL.h>
	#endif
THIRD_PARTY_INCLUDES_END

// Based on https://github.com/bminor/SDL/blob/master/src/joystick/controller_type.h

#define MAKE_CONTROLLER_ID(VendorId, ProductId) (uint32)(VendorId << 16 | ProductId)
typedef struct
{
	uint32 DeviceId;
	EEstControllerType ControllerType;
} TEstControllerDescription;

static const TArray<TEstControllerDescription> ControllerDescriptions =
{
	{ MAKE_CONTROLLER_ID(0x0079, 0x18d4), EEstControllerType::XBox360Controller },	// GPD Win 2 X-Box Controller
	{ MAKE_CONTROLLER_ID(0x044f, 0xb326), EEstControllerType::XBox360Controller },	// Thrustmaster Gamepad GP XID
	{ MAKE_CONTROLLER_ID(0x045e, 0x028e), EEstControllerType::XBox360Controller },	// Microsoft X-Box 360 pad
	{ MAKE_CONTROLLER_ID(0x045e, 0x028f), EEstControllerType::XBox360Controller },	// Microsoft X-Box 360 pad v2
	{ MAKE_CONTROLLER_ID(0x045e, 0x0291), EEstControllerType::XBox360Controller },	// Xbox 360 Wireless Receiver (XBOX)
	{ MAKE_CONTROLLER_ID(0x045e, 0x02a0), EEstControllerType::XBox360Controller },	// Microsoft X-Box 360 Big Button IR
	{ MAKE_CONTROLLER_ID(0x045e, 0x02a1), EEstControllerType::XBox360Controller },	// Microsoft X-Box 360 pad
	{ MAKE_CONTROLLER_ID(0x045e, 0x02d1), EEstControllerType::XBoxOneController },	// Microsoft X-Box One pad
	{ MAKE_CONTROLLER_ID(0x045e, 0x02dd), EEstControllerType::XBoxOneController },	// Microsoft X-Box One pad (Firmware 2015)
	{ MAKE_CONTROLLER_ID(0x045e, 0x02e0), EEstControllerType::XBoxOneController },	// Microsoft X-Box One S pad (Bluetooth)
	{ MAKE_CONTROLLER_ID(0x045e, 0x02e3), EEstControllerType::XBoxOneController },	// Microsoft X-Box One Elite pad
	{ MAKE_CONTROLLER_ID(0x045e, 0x02ea), EEstControllerType::XBoxOneController },	// Microsoft X-Box One S pad
	{ MAKE_CONTROLLER_ID(0x045e, 0x02fd), EEstControllerType::XBoxOneController },	// Microsoft X-Box One S pad (Bluetooth)
	{ MAKE_CONTROLLER_ID(0x045e, 0x02ff), EEstControllerType::XBoxOneController },	// Microsoft X-Box One Elite pad
	{ MAKE_CONTROLLER_ID(0x045e, 0x0719), EEstControllerType::XBox360Controller },	// Xbox 360 Wireless Receiver
	{ MAKE_CONTROLLER_ID(0x046d, 0xc21d), EEstControllerType::XBox360Controller },	// Logitech Gamepad F310
	{ MAKE_CONTROLLER_ID(0x046d, 0xc21e), EEstControllerType::XBox360Controller },	// Logitech Gamepad F510
	{ MAKE_CONTROLLER_ID(0x046d, 0xc21f), EEstControllerType::XBox360Controller },	// Logitech Gamepad F710
	{ MAKE_CONTROLLER_ID(0x046d, 0xc242), EEstControllerType::XBox360Controller },	// Logitech Chillstream Controller

	{ MAKE_CONTROLLER_ID(0x054c, 0x0268), EEstControllerType::PS3Controller },		// Sony PS3 Controller
	{ MAKE_CONTROLLER_ID(0x0925, 0x0005), EEstControllerType::PS3Controller },		// Sony PS3 Controller
	{ MAKE_CONTROLLER_ID(0x8888, 0x0308), EEstControllerType::PS3Controller },		// Sony PS3 Controller
	{ MAKE_CONTROLLER_ID(0x1a34, 0x0836), EEstControllerType::PS3Controller },		// Afterglow ps3
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x006e), EEstControllerType::PS3Controller },		// HORI horipad4 ps3
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0066), EEstControllerType::PS3Controller },		// HORI horipad4 ps4
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x005f), EEstControllerType::PS3Controller },		// HORI Fighting commander ps3
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x005e), EEstControllerType::PS3Controller },		// HORI Fighting commander ps4
	//{ MAKE_CONTROLLER_ID( 0x0738, 0x3250 ), EEstControllerType::PS3Controller },		// madcats fightpad pro ps3 already in ps4 list.. does this work??
	{ MAKE_CONTROLLER_ID(0x0738, 0x8250), EEstControllerType::PS3Controller },		// madcats fightpad pro ps4
	{ MAKE_CONTROLLER_ID(0x0079, 0x181a), EEstControllerType::PS3Controller },		// Venom Arcade Stick
	{ MAKE_CONTROLLER_ID(0x0079, 0x0006), EEstControllerType::PS3Controller },		// PC Twin Shock Controller - looks like a DS3 but the face buttons are 1-4 instead of symbols
	{ MAKE_CONTROLLER_ID(0x0079, 0x1844), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x8888, 0x0308), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x2563, 0x0575), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x0810, 0x0001), EEstControllerType::PS3Controller },		// actually ps2 - maybe break out later
	{ MAKE_CONTROLLER_ID(0x0810, 0x0003), EEstControllerType::PS3Controller },		// actually ps2 - maybe break out later
	{ MAKE_CONTROLLER_ID(0x2563, 0x0523), EEstControllerType::PS3Controller },		// Digiflip GP006
	{ MAKE_CONTROLLER_ID(0x11ff, 0x3331), EEstControllerType::PS3Controller },		// SRXJ-PH2400
	{ MAKE_CONTROLLER_ID(0x20bc, 0x5500), EEstControllerType::PS3Controller },		// ShanWan PS3
	{ MAKE_CONTROLLER_ID(0x05b8, 0x1004), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x146b, 0x0603), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x044f, 0xb315), EEstControllerType::PS3Controller },		// Firestorm Dual Analog 3
	{ MAKE_CONTROLLER_ID(0x0925, 0x8888), EEstControllerType::PS3Controller },		// Actually ps2 -maybe break out later Lakeview Research WiseGroup Ltd, MP-8866 Dual Joypad
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x004d), EEstControllerType::PS3Controller },		// Horipad 3
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0009), EEstControllerType::PS3Controller },		// HORI BDA GP1
	{ MAKE_CONTROLLER_ID(0x0e8f, 0x0008), EEstControllerType::PS3Controller },		// Green Asia
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x006a), EEstControllerType::PS3Controller },		// Real Arcade Pro 4
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x011e), EEstControllerType::PS3Controller },		// Rock Candy PS4
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0214), EEstControllerType::PS3Controller },		// afterglow ps3
	{ MAKE_CONTROLLER_ID(0x0925, 0x8866), EEstControllerType::PS3Controller },		// PS2 maybe break out later
	{ MAKE_CONTROLLER_ID(0x0e8f, 0x310d), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x2c22, 0x2003), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x056e, 0x2013), EEstControllerType::PS3Controller },		// JC-U4113SBK
	{ MAKE_CONTROLLER_ID(0x0738, 0x8838), EEstControllerType::PS3Controller },		// Madcatz Fightstick Pro
	{ MAKE_CONTROLLER_ID(0x1a34, 0x0836), EEstControllerType::PS3Controller },		// Afterglow PS3
	{ MAKE_CONTROLLER_ID(0x0f30, 0x1100), EEstControllerType::PS3Controller },		// Quanba Q1 fight stick
	{ MAKE_CONTROLLER_ID(0x1345, 0x6005), EEstControllerType::PS3Controller },		// ps2 maybe break out later
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0087), EEstControllerType::PS3Controller },		// HORI fighting mini stick
	{ MAKE_CONTROLLER_ID(0x146b, 0x5500), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x20d6, 0xca6d), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x25f0, 0xc121), EEstControllerType::PS3Controller },		//
	{ MAKE_CONTROLLER_ID(0x8380, 0x0003), EEstControllerType::PS3Controller },		// BTP 2163
	{ MAKE_CONTROLLER_ID(0x1345, 0x1000), EEstControllerType::PS3Controller },		// PS2 ACME GA-D5
	{ MAKE_CONTROLLER_ID(0x0e8f, 0x3075), EEstControllerType::PS3Controller },		// SpeedLink Strike FX
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0128), EEstControllerType::PS3Controller },		// Rock Candy PS3
	{ MAKE_CONTROLLER_ID(0x2c22, 0x2000), EEstControllerType::PS3Controller },		// Quanba Drone
	{ MAKE_CONTROLLER_ID(0x06a3, 0xf622), EEstControllerType::PS3Controller },		// Cyborg V3
	{ MAKE_CONTROLLER_ID(0x044f, 0xd007), EEstControllerType::PS3Controller },		// Thrustmaster wireless 3-1
	{ MAKE_CONTROLLER_ID(0x25f0, 0x83c3), EEstControllerType::PS3Controller },		// gioteck vx2
	{ MAKE_CONTROLLER_ID(0x05b8, 0x1006), EEstControllerType::PS3Controller },		// JC-U3412SBK
	{ MAKE_CONTROLLER_ID(0x20d6, 0x576d), EEstControllerType::PS3Controller },		// Power A PS3
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x6302), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x056e, 0x200f), EEstControllerType::PS3Controller },		// From SDL
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x1314), EEstControllerType::PS3Controller },		// PDP Afterglow Wireless PS3 controller

	{ MAKE_CONTROLLER_ID(0x054c, 0x05c4), EEstControllerType::PS4Controller },		// Sony PS4 Controller
	{ MAKE_CONTROLLER_ID(0x054c, 0x09cc), EEstControllerType::PS4Controller },		// Sony PS4 Slim Controller
	{ MAKE_CONTROLLER_ID(0x054c, 0x0ba0), EEstControllerType::PS4Controller },		// Sony PS4 Controller (Wireless dongle)
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x008a), EEstControllerType::PS4Controller },		// HORI Real Arcade Pro 4
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0055), EEstControllerType::PS4Controller },		// HORIPAD 4 FPS
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0066), EEstControllerType::PS4Controller },		// HORIPAD 4 FPS Plus 
	{ MAKE_CONTROLLER_ID(0x0738, 0x8384), EEstControllerType::PS4Controller },		// Mad Catz FightStick TE S+ PS4
	{ MAKE_CONTROLLER_ID(0x0738, 0x8250), EEstControllerType::PS4Controller },		// Mad Catz FightPad Pro PS4
	{ MAKE_CONTROLLER_ID(0x0C12, 0x0E10), EEstControllerType::PS4Controller },		// Armor Armor 3 Pad PS4
	{ MAKE_CONTROLLER_ID(0x0C12, 0x1CF6), EEstControllerType::PS4Controller },		// EMIO PS4 Elite Controller
	{ MAKE_CONTROLLER_ID(0x1532, 0x1000), EEstControllerType::PS4Controller },		// Razer Raiju PS4 Controller
	{ MAKE_CONTROLLER_ID(0x1532, 0X0401), EEstControllerType::PS4Controller },		// Razer Panthera PS4 Controller
	{ MAKE_CONTROLLER_ID(0x054c, 0x05c5), EEstControllerType::PS4Controller },		// STRIKEPAD PS4 Grip Add-on
	{ MAKE_CONTROLLER_ID(0x146b, 0x0d01), EEstControllerType::PS4Controller },		// Nacon Revolution Pro Controller - has gyro
	{ MAKE_CONTROLLER_ID(0x146b, 0x0d02), EEstControllerType::PS4Controller },		// Nacon Revolution Pro Controller v2 - has gyro
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x00a0), EEstControllerType::PS4Controller },		// HORI TAC4 mousething
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x009c), EEstControllerType::PS4Controller },		// HORI TAC PRO mousething
	{ MAKE_CONTROLLER_ID(0x0c12, 0x0ef6), EEstControllerType::PS4Controller },		// Hitbox Arcade Stick
	{ MAKE_CONTROLLER_ID(0x0079, 0x181b), EEstControllerType::PS4Controller },		// Venom Arcade Stick - XXX:this may not work and may need to be called a ps3 controller
	{ MAKE_CONTROLLER_ID(0x0738, 0x3250), EEstControllerType::PS4Controller },		// Mad Catz FightPad PRO - controller shaped with 6 face buttons
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x00ee), EEstControllerType::PS4Controller },		// Hori mini wired https://www.playstation.com/en-us/explore/accessories/gaming-controllers/mini-wired-gamepad/
	{ MAKE_CONTROLLER_ID(0x0738, 0x8481), EEstControllerType::PS4Controller },		// Mad Catz FightStick TE 2+ PS4
	{ MAKE_CONTROLLER_ID(0x0738, 0x8480), EEstControllerType::PS4Controller },		// Mad Catz FightStick TE 2 PS4
	{ MAKE_CONTROLLER_ID(0x7545, 0x0104), EEstControllerType::PS4Controller },		// Armor 3 or Level Up Cobra - At least one variant has gyro
	{ MAKE_CONTROLLER_ID(0x0c12, 0x0e15), EEstControllerType::PS4Controller },		// Game:Pad 4
	{ MAKE_CONTROLLER_ID(0x11c0, 0x4001), EEstControllerType::PS4Controller },		// "PS4 Fun Controller" added from user log

	{ MAKE_CONTROLLER_ID(0x1532, 0x1007), EEstControllerType::PS4Controller },		// Razer Raiju 2 Tournament edition USB- untested and added for razer
	{ MAKE_CONTROLLER_ID(0x1532, 0x100A), EEstControllerType::PS4Controller },		// Razer Raiju 2 Tournament edition BT - untested and added for razer
	{ MAKE_CONTROLLER_ID(0x1532, 0x1004), EEstControllerType::PS4Controller },		// Razer Raiju 2 Ultimate USB - untested and added for razer
	{ MAKE_CONTROLLER_ID(0x1532, 0x1009), EEstControllerType::PS4Controller },		// Razer Raiju 2 Ultimate BT - untested and added for razer
	{ MAKE_CONTROLLER_ID(0x1532, 0x1008), EEstControllerType::PS4Controller },		// Razer Panthera Evo Fightstick - untested and added for razer

	{ MAKE_CONTROLLER_ID(0x056e, 0x2004), EEstControllerType::XBox360Controller },	// Elecom JC-U3613M
	{ MAKE_CONTROLLER_ID(0x06a3, 0xf51a), EEstControllerType::XBox360Controller },	// Saitek P3600
	{ MAKE_CONTROLLER_ID(0x0738, 0x4716), EEstControllerType::XBox360Controller },	// Mad Catz Wired Xbox 360 Controller
	{ MAKE_CONTROLLER_ID(0x0738, 0x4718), EEstControllerType::XBox360Controller },	// Mad Catz Street Fighter IV FightStick SE
	{ MAKE_CONTROLLER_ID(0x0738, 0x4726), EEstControllerType::XBox360Controller },	// Mad Catz Xbox 360 Controller
	{ MAKE_CONTROLLER_ID(0x0738, 0x4728), EEstControllerType::XBox360Controller },	// Mad Catz Street Fighter IV FightPad
	{ MAKE_CONTROLLER_ID(0x0738, 0x4736), EEstControllerType::XBox360Controller },	// Mad Catz MicroCon Gamepad
	{ MAKE_CONTROLLER_ID(0x0738, 0x4738), EEstControllerType::XBox360Controller },	// Mad Catz Wired Xbox 360 Controller (SFIV)
	{ MAKE_CONTROLLER_ID(0x0738, 0x4740), EEstControllerType::XBox360Controller },	// Mad Catz Beat Pad
	{ MAKE_CONTROLLER_ID(0x0738, 0x4a01), EEstControllerType::XBoxOneController },	// Mad Catz FightStick TE 2
	{ MAKE_CONTROLLER_ID(0x0738, 0xb726), EEstControllerType::XBox360Controller },	// Mad Catz Xbox controller - MW2
	{ MAKE_CONTROLLER_ID(0x0738, 0xbeef), EEstControllerType::XBox360Controller },	// Mad Catz JOYTECH NEO SE Advanced GamePad
	{ MAKE_CONTROLLER_ID(0x0738, 0xcb02), EEstControllerType::XBox360Controller },	// Saitek Cyborg Rumble Pad - PC/Xbox 360
	{ MAKE_CONTROLLER_ID(0x0738, 0xcb03), EEstControllerType::XBox360Controller },	// Saitek P3200 Rumble Pad - PC/Xbox 360
	{ MAKE_CONTROLLER_ID(0x0738, 0xf738), EEstControllerType::XBox360Controller },	// Super SFIV FightStick TE S
	{ MAKE_CONTROLLER_ID(0x0955, 0xb400), EEstControllerType::XBox360Controller },	// NVIDIA Shield streaming controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0105), EEstControllerType::XBox360Controller },	// HSM3 Xbox360 dancepad
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0113), EEstControllerType::XBox360Controller },	// Afterglow AX.1 Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x011f), EEstControllerType::XBox360Controller },	// Rock Candy Gamepad Wired Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0131), EEstControllerType::XBox360Controller },	// PDP EA Sports Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0133), EEstControllerType::XBox360Controller },	// Xbox 360 Wired Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0139), EEstControllerType::XBoxOneController },	// Afterglow Prismatic Wired Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x013a), EEstControllerType::XBoxOneController },	// PDP Xbox One Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0146), EEstControllerType::XBoxOneController },	// Rock Candy Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0147), EEstControllerType::XBoxOneController },	// PDP Marvel Xbox One Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x015c), EEstControllerType::XBoxOneController },	// PDP Xbox One Arcade Stick
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0161), EEstControllerType::XBoxOneController },	// PDP Xbox One Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0162), EEstControllerType::XBoxOneController },	// PDP Xbox One Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0163), EEstControllerType::XBoxOneController },	// PDP Xbox One Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0164), EEstControllerType::XBoxOneController },	// PDP Battlefield One
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0165), EEstControllerType::XBoxOneController },	// PDP Titanfall 2
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0201), EEstControllerType::XBox360Controller },	// Pelican PL-3601 'TSZ' Wired Xbox 360 Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0213), EEstControllerType::XBox360Controller },	// Afterglow Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x021f), EEstControllerType::XBox360Controller },	// Rock Candy Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0246), EEstControllerType::XBoxOneController },	// Rock Candy Gamepad for Xbox One 2015
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x02a0), EEstControllerType::XBox360Controller },	// Counterfeit 360Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0301), EEstControllerType::XBox360Controller },	// Logic3 Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0346), EEstControllerType::XBoxOneController },	// Rock Candy Gamepad for Xbox One 2016
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0401), EEstControllerType::XBox360Controller },	// Logic3 Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0413), EEstControllerType::XBox360Controller },	// Afterglow AX.1 Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0501), EEstControllerType::XBox360Controller },	// PDP Xbox 360 Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0xf501), EEstControllerType::XBox360Controller },	// Counterfeit 360 Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0xf900), EEstControllerType::XBox360Controller },	// PDP Afterglow AX.1
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x000a), EEstControllerType::XBox360Controller },	// Hori Co. DOA4 FightStick
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x000c), EEstControllerType::XBox360Controller },	// Hori PadEX Turbo
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x000d), EEstControllerType::XBox360Controller },	// Hori Fighting Stick EX2
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0016), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro.EX
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x001b), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro VX
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0063), EEstControllerType::XBoxOneController },	// Hori Real Arcade Pro Hayabusa (USA) Xbox One
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0067), EEstControllerType::XBoxOneController },	// HORIPAD ONE
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0078), EEstControllerType::XBoxOneController },	// Hori Real Arcade Pro V Kai Xbox One
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x008c), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro 4
	{ MAKE_CONTROLLER_ID(0x11c9, 0x55f0), EEstControllerType::XBox360Controller },	// Nacon GC-100XF
	{ MAKE_CONTROLLER_ID(0x12ab, 0x0004), EEstControllerType::XBox360Controller },	// Honey Bee Xbox360 dancepad
	{ MAKE_CONTROLLER_ID(0x12ab, 0x0301), EEstControllerType::XBox360Controller },	// PDP AFTERGLOW AX.1
	{ MAKE_CONTROLLER_ID(0x12ab, 0x0303), EEstControllerType::XBox360Controller },	// Mortal Kombat Klassic FightStick
	{ MAKE_CONTROLLER_ID(0x1430, 0x02a0), EEstControllerType::XBox360Controller },	// RedOctane Controller Adapter
	{ MAKE_CONTROLLER_ID(0x1430, 0x4748), EEstControllerType::XBox360Controller },	// RedOctane Guitar Hero X-plorer
	{ MAKE_CONTROLLER_ID(0x1430, 0xf801), EEstControllerType::XBox360Controller },	// RedOctane Controller
	{ MAKE_CONTROLLER_ID(0x146b, 0x0601), EEstControllerType::XBox360Controller },	// BigBen Interactive XBOX 360 Controller
	{ MAKE_CONTROLLER_ID(0x1532, 0x0037), EEstControllerType::XBox360Controller },	// Razer Sabertooth
	{ MAKE_CONTROLLER_ID(0x1532, 0x0a00), EEstControllerType::XBoxOneController },	// Razer Atrox Arcade Stick
	{ MAKE_CONTROLLER_ID(0x1532, 0x0a03), EEstControllerType::XBoxOneController },	// Razer Wildcat
	{ MAKE_CONTROLLER_ID(0x15e4, 0x3f00), EEstControllerType::XBox360Controller },	// Power A Mini Pro Elite
	{ MAKE_CONTROLLER_ID(0x15e4, 0x3f0a), EEstControllerType::XBox360Controller },	// Xbox Airflo wired controller
	{ MAKE_CONTROLLER_ID(0x15e4, 0x3f10), EEstControllerType::XBox360Controller },	// Batarang Xbox 360 controller
	{ MAKE_CONTROLLER_ID(0x162e, 0xbeef), EEstControllerType::XBox360Controller },	// Joytech Neo-Se Take2
	{ MAKE_CONTROLLER_ID(0x1689, 0xfd00), EEstControllerType::XBox360Controller },	// Razer Onza Tournament Edition
	{ MAKE_CONTROLLER_ID(0x1689, 0xfd01), EEstControllerType::XBox360Controller },	// Razer Onza Classic Edition
	{ MAKE_CONTROLLER_ID(0x1689, 0xfe00), EEstControllerType::XBox360Controller },	// Razer Sabertooth
	{ MAKE_CONTROLLER_ID(0x1bad, 0x0002), EEstControllerType::XBox360Controller },	// Harmonix Rock Band Guitar
	{ MAKE_CONTROLLER_ID(0x1bad, 0x0003), EEstControllerType::XBox360Controller },	// Harmonix Rock Band Drumkit
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf016), EEstControllerType::XBox360Controller },	// Mad Catz Xbox 360 Controller
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf018), EEstControllerType::XBox360Controller },	// Mad Catz Street Fighter IV SE Fighting Stick
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf019), EEstControllerType::XBox360Controller },	// Mad Catz Brawlstick for Xbox 360
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf021), EEstControllerType::XBox360Controller },	// Mad Cats Ghost Recon FS GamePad
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf023), EEstControllerType::XBox360Controller },	// MLG Pro Circuit Controller (Xbox)
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf025), EEstControllerType::XBox360Controller },	// Mad Catz Call Of Duty
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf027), EEstControllerType::XBox360Controller },	// Mad Catz FPS Pro
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf028), EEstControllerType::XBox360Controller },	// Street Fighter IV FightPad
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf02e), EEstControllerType::XBox360Controller },	// Mad Catz Fightpad
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf036), EEstControllerType::XBox360Controller },	// Mad Catz MicroCon GamePad Pro
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf038), EEstControllerType::XBox360Controller },	// Street Fighter IV FightStick TE
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf039), EEstControllerType::XBox360Controller },	// Mad Catz MvC2 TE
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf03a), EEstControllerType::XBox360Controller },	// Mad Catz SFxT Fightstick Pro
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf03d), EEstControllerType::XBox360Controller },	// Street Fighter IV Arcade Stick TE - Chun Li
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf03e), EEstControllerType::XBox360Controller },	// Mad Catz MLG FightStick TE
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf03f), EEstControllerType::XBox360Controller },	// Mad Catz FightStick SoulCaliber
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf042), EEstControllerType::XBox360Controller },	// Mad Catz FightStick TES+
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf080), EEstControllerType::XBox360Controller },	// Mad Catz FightStick TE2
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf501), EEstControllerType::XBox360Controller },	// HoriPad EX2 Turbo
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf502), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro.VX SA
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf503), EEstControllerType::XBox360Controller },	// Hori Fighting Stick VX
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf504), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro. EX
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf505), EEstControllerType::XBox360Controller },	// Hori Fighting Stick EX2B
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf506), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro.EX Premium VLX
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf900), EEstControllerType::XBox360Controller },	// Harmonix Xbox 360 Controller
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf901), EEstControllerType::XBox360Controller },	// Gamestop Xbox 360 Controller
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf902), EEstControllerType::XBox360Controller },	// Mad Catz Gamepad2
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf903), EEstControllerType::XBox360Controller },	// Tron Xbox 360 controller
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf904), EEstControllerType::XBox360Controller },	// PDP Versus Fighting Pad
	{ MAKE_CONTROLLER_ID(0x1bad, 0xf906), EEstControllerType::XBox360Controller },	// MortalKombat FightStick
	{ MAKE_CONTROLLER_ID(0x1bad, 0xfa01), EEstControllerType::XBox360Controller },	// MadCatz GamePad
	{ MAKE_CONTROLLER_ID(0x1bad, 0xfd00), EEstControllerType::XBox360Controller },	// Razer Onza TE
	{ MAKE_CONTROLLER_ID(0x1bad, 0xfd01), EEstControllerType::XBox360Controller },	// Razer Onza
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5000), EEstControllerType::XBox360Controller },	// Razer Atrox Arcade Stick
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5300), EEstControllerType::XBox360Controller },	// PowerA MINI PROEX Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5303), EEstControllerType::XBox360Controller },	// Xbox Airflo wired controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x530a), EEstControllerType::XBox360Controller },	// Xbox 360 Pro EX Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x531a), EEstControllerType::XBox360Controller },	// PowerA Pro Ex
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5397), EEstControllerType::XBox360Controller },	// FUS1ON Tournament Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x541a), EEstControllerType::XBoxOneController },	// PowerA Xbox One Mini Wired Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x542a), EEstControllerType::XBoxOneController },	// Xbox ONE spectra
	{ MAKE_CONTROLLER_ID(0x24c6, 0x543a), EEstControllerType::XBoxOneController },	// PowerA Xbox One wired controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5500), EEstControllerType::XBox360Controller },	// Hori XBOX 360 EX 2 with Turbo
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5501), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro VX-SA
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5502), EEstControllerType::XBox360Controller },	// Hori Fighting Stick VX Alt
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5503), EEstControllerType::XBox360Controller },	// Hori Fighting Edge
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5506), EEstControllerType::XBox360Controller },	// Hori SOULCALIBUR V Stick
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5510), EEstControllerType::XBox360Controller },	// Hori Fighting Commander ONE
	{ MAKE_CONTROLLER_ID(0x24c6, 0x550d), EEstControllerType::XBox360Controller },	// Hori GEM Xbox controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x550e), EEstControllerType::XBox360Controller },	// Hori Real Arcade Pro V Kai 360
	{ MAKE_CONTROLLER_ID(0x24c6, 0x551a), EEstControllerType::XBoxOneController },	// PowerA FUSION Pro Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x561a), EEstControllerType::XBoxOneController },	// PowerA FUSION Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5b00), EEstControllerType::XBox360Controller },	// ThrustMaster Ferrari Italia 458 Racing Wheel
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5b02), EEstControllerType::XBox360Controller },	// Thrustmaster, Inc. GPX Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5b03), EEstControllerType::XBox360Controller },	// Thrustmaster Ferrari 458 Racing Wheel
	{ MAKE_CONTROLLER_ID(0x24c6, 0x5d04), EEstControllerType::XBox360Controller },	// Razer Sabertooth
	{ MAKE_CONTROLLER_ID(0x24c6, 0xfafa), EEstControllerType::XBox360Controller },	// Aplay Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0xfafb), EEstControllerType::XBox360Controller },	// Aplay Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0xfafc), EEstControllerType::XBox360Controller },	// Afterglow Gamepad 1
	{ MAKE_CONTROLLER_ID(0x24c6, 0xfafe), EEstControllerType::XBox360Controller },	// Rock Candy Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID(0x24c6, 0xfafd), EEstControllerType::XBox360Controller },	// Afterglow Gamepad 3

	// These have been added via Minidump for unrecognized Xinput controller assert
	{ MAKE_CONTROLLER_ID(0x0000, 0x0000), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x045e, 0x02a2), EEstControllerType::XBox360Controller },	// Unknown Controller - Microsoft VID
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x1414), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x1314), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0e6f, 0x0159), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x24c6, 0xfaff), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0086), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x006d), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x00a4), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x1832), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x187f), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x1883), EEstControllerType::XBox360Controller },	// Unknown Controller	
	{ MAKE_CONTROLLER_ID(0x03eb, 0xff01), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x2c22, 0x2303), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0c12, 0x0ef8), EEstControllerType::XBox360Controller },	// Homemade fightstick based on brook pcb (with XInput driver??)
	{ MAKE_CONTROLLER_ID(0x046d, 0x1000), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x1345, 0x6006), EEstControllerType::XBox360Controller },	// Unknown Controller

	{ MAKE_CONTROLLER_ID(0x056e, 0x2012), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x146b, 0x0602), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x00ae), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x146b, 0x0603), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x056e, 0x2013), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x046d, 0x0401), EEstControllerType::XBox360Controller },	// logitech xinput
	{ MAKE_CONTROLLER_ID(0x046d, 0x0301), EEstControllerType::XBox360Controller },	// logitech xinput
	{ MAKE_CONTROLLER_ID(0x046d, 0xcaa3), EEstControllerType::XBox360Controller },	// logitech xinput
	{ MAKE_CONTROLLER_ID(0x046d, 0xc261), EEstControllerType::XBox360Controller },	// logitech xinput
	{ MAKE_CONTROLLER_ID(0x046d, 0x0291), EEstControllerType::XBox360Controller },	// logitech xinput
	{ MAKE_CONTROLLER_ID(0x0079, 0x18d3), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x00b1), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0001, 0x0001), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x1345, 0x6005), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x188e), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x18d4), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x2c22, 0x2003), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x00b1), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x187c), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x189c), EEstControllerType::XBox360Controller },	// Unknown Controller
	{ MAKE_CONTROLLER_ID(0x0079, 0x1874), EEstControllerType::XBox360Controller },	// Unknown Controller

	{ MAKE_CONTROLLER_ID(0x1038, 0xb360), EEstControllerType::XBox360Controller },	// SteelSeries Nimbus/Stratus XL


	//{ MAKE_CONTROLLER_ID( 0x1949, 0x0402 ), /*android*/ },	// Unknown Controller

	{ MAKE_CONTROLLER_ID(0x05ac, 0x0001), EEstControllerType::AppleController },	// MFI Extended Gamepad (generic entry for iOS/tvOS)
	{ MAKE_CONTROLLER_ID(0x05ac, 0x0002), EEstControllerType::AppleController },	// MFI Standard Gamepad (generic entry for iOS/tvOS)

	// We currently don't support using a pair of Switch Joy-Con's as a single
	// controller and we don't want to support using them individually for the
	// time being, so these should be disabled until one of the above is true
	// { MAKE_CONTROLLER_ID( 0x057e, 0x2006 ), EEstControllerType::SwitchJoyConLeft },    // Nintendo Switch Joy-Con (Left)
	// { MAKE_CONTROLLER_ID( 0x057e, 0x2007 ), EEstControllerType::SwitchJoyConRight },   // Nintendo Switch Joy-Con (Right)

	// This same controller ID is spoofed by many 3rd-party Switch controllers.
	// The ones we currently know of are:
	// * Any 8bitdo controller with Switch support
	// * ORTZ Gaming Wireless Pro Controller
	// * ZhiXu Gamepad Wireless
	// * Sunwaytek Wireless Motion Controller for Nintendo Switch
	{ MAKE_CONTROLLER_ID(0x057e, 0x2009), EEstControllerType::SwitchProController },        // Nintendo Switch Pro Controller

	{ MAKE_CONTROLLER_ID(0x0f0d, 0x00c1), EEstControllerType::SwitchInputOnlyController },  // HORIPAD for Nintendo Switch
	{ MAKE_CONTROLLER_ID(0x20d6, 0xa711), EEstControllerType::SwitchInputOnlyController },  // PowerA Wired Controller Plus
	{ MAKE_CONTROLLER_ID(0x0f0d, 0x0092), EEstControllerType::SwitchInputOnlyController },  // HORI Pokken Tournament DX Pro Pad

	{ MAKE_CONTROLLER_ID(0x057e, 0x0337), EEstControllerType::GameCube }, // Nintendo Wii U/Switch GameCube USB Adapter


	// Valve products - don't add to public list
	{ MAKE_CONTROLLER_ID(0x0000, 0x11fb), EEstControllerType::MobileTouch },		// Streaming mobile touch virtual controls
	{ MAKE_CONTROLLER_ID(0x28de, 0x1101), EEstControllerType::SteamController },	// Valve Legacy Steam Controller (CHELL)
	{ MAKE_CONTROLLER_ID(0x28de, 0x1102), EEstControllerType::SteamController },	// Valve wired Steam Controller (D0G)
	{ MAKE_CONTROLLER_ID(0x28de, 0x1105), EEstControllerType::SteamControllerV2 },	// Valve Bluetooth Steam Controller (D0G)
	{ MAKE_CONTROLLER_ID(0x28de, 0x1106), EEstControllerType::SteamControllerV2 },	// Valve Bluetooth Steam Controller (D0G)
	{ MAKE_CONTROLLER_ID(0x28de, 0x1142), EEstControllerType::SteamController },	// Valve wireless Steam Controller
	{ MAKE_CONTROLLER_ID(0x28de, 0x1201), EEstControllerType::SteamController },	// Valve wired Steam Controller (HEADCRAB)
};

TArray<EEstControllerType> UEstInputStatics::GetConnectedControllerTypes()
{
	TSet<EEstControllerType> ControllerTypes;

#if PLATFORM_WINDOWS
	SDL_JoystickUpdate();

	for (int32 i = 0; i < SDL_NumJoysticks(); ++i)
	{
		uint16 VendorId = SDL_JoystickGetDeviceVendor(i);
		uint16 ProductId = SDL_JoystickGetDeviceProduct(i);
		uint32 ControllerId = MAKE_CONTROLLER_ID(VendorId, ProductId);

		for (TEstControllerDescription ControllerDescription : ControllerDescriptions)
		{
			if (ControllerDescription.DeviceId == ControllerId)
			{
				ControllerTypes.Add(ControllerDescription.ControllerType);
				break;
			}
		}
	}
#endif

	return ControllerTypes.Array();
}

#undef MAKE_CONTROLLER_ID
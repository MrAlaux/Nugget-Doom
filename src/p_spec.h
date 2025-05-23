//
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:  definitions, declarations and prototypes for specials
//
//-----------------------------------------------------------------------------

#ifndef __P_SPEC__
#define __P_SPEC__

#include "d_think.h"
#include "doomdef.h"
#include "doomtype.h"
#include "m_fixed.h"

struct line_s;
struct mobj_s;
struct player_s;
struct sector_s;

// [Nugget] CVARs
extern boolean comp_manualdoor;
extern boolean comp_switchsource;
extern boolean comp_keynoway;

//      Define values for map objects
#define MO_TELEPORTMAN  14

// p_floor

#define ELEVATORSPEED (FRACUNIT*4)
#define FLOORSPEED     FRACUNIT

// p_ceilng

#define CEILSPEED   FRACUNIT
#define CEILWAIT    150

// p_doors

#define VDOORSPEED  (FRACUNIT*2)
#define VDOORWAIT   150

// p_plats

#define PLATWAIT    3

#define PLATSPEED   (beta_emulation ? FRACUNIT/3 : FRACUNIT)

// p_switch

// 4 players, 4 buttons each at once, max.
// killough 2/14/98: redefine in terms of MAXPLAYERS
#define MAXBUTTONS    (MAXPLAYERS*4)

// 1 second, in ticks. 
#define BUTTONTIME  TICRATE

// p_lights

#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35

//jff 3/14/98 add bits and shifts for generalized sector types

#define DAMAGE_MASK     0x60
#define DAMAGE_SHIFT    5
#define SECRET_MASK     0x80
#define SECRET_SHIFT    7
#define FRICTION_MASK   0x100
#define FRICTION_SHIFT  8
#define PUSH_MASK       0x200
#define PUSH_SHIFT      9

// mbf21
#define DEATH_MASK            0x1000 // bit 12
#define KILL_MONSTERS_MASK    0x2000 // bit 13

//jff 02/04/98 Define masks, shifts, for fields in 
// generalized linedef types

#define GenFloorBase          0x6000
#define GenCeilingBase        0x4000
#define GenDoorBase           0x3c00
#define GenLockedBase         0x3800
#define GenLiftBase           0x3400
#define GenStairsBase         0x3000
#define GenCrusherBase        0x2F80

#define TriggerType           0x0007
#define TriggerTypeShift      0

// define masks and shifts for the floor type fields

#define FloorCrush            0x1000
#define FloorChange           0x0c00
#define FloorTarget           0x0380
#define FloorDirection        0x0040
#define FloorModel            0x0020
#define FloorSpeed            0x0018

#define FloorCrushShift           12
#define FloorChangeShift          10
#define FloorTargetShift           7
#define FloorDirectionShift        6
#define FloorModelShift            5
#define FloorSpeedShift            3
                               
// define masks and shifts for the ceiling type fields

#define CeilingCrush          0x1000
#define CeilingChange         0x0c00
#define CeilingTarget         0x0380
#define CeilingDirection      0x0040
#define CeilingModel          0x0020
#define CeilingSpeed          0x0018

#define CeilingCrushShift         12
#define CeilingChangeShift        10
#define CeilingTargetShift         7
#define CeilingDirectionShift      6
#define CeilingModelShift          5
#define CeilingSpeedShift          3

// define masks and shifts for the lift type fields

#define LiftTarget            0x0300
#define LiftDelay             0x00c0
#define LiftMonster           0x0020
#define LiftSpeed             0x0018

#define LiftTargetShift            8
#define LiftDelayShift             6
#define LiftMonsterShift           5
#define LiftSpeedShift             3

// define masks and shifts for the stairs type fields

#define StairIgnore           0x0200
#define StairDirection        0x0100
#define StairStep             0x00c0
#define StairMonster          0x0020
#define StairSpeed            0x0018

#define StairIgnoreShift           9
#define StairDirectionShift        8
#define StairStepShift             6
#define StairMonsterShift          5
#define StairSpeedShift            3

// define masks and shifts for the crusher type fields

#define CrusherSilent         0x0040
#define CrusherMonster        0x0020
#define CrusherSpeed          0x0018

#define CrusherSilentShift         6
#define CrusherMonsterShift        5
#define CrusherSpeedShift          3

// define masks and shifts for the door type fields

#define DoorDelay             0x0300
#define DoorMonster           0x0080
#define DoorKind              0x0060
#define DoorSpeed             0x0018

#define DoorDelayShift             8
#define DoorMonsterShift           7
#define DoorKindShift              5
#define DoorSpeedShift             3

// define masks and shifts for the locked door type fields

#define LockedNKeys           0x0200
#define LockedKey             0x01c0
#define LockedKind            0x0020
#define LockedSpeed           0x0018

#define LockedNKeysShift           9
#define LockedKeyShift             6
#define LockedKindShift            5
#define LockedSpeedShift           3

// define names for the TriggerType field of the general linedefs

typedef enum
{
  WalkOnce,
  WalkMany,
  SwitchOnce,
  SwitchMany,
  GunOnce,
  GunMany,
  PushOnce,
  PushMany,
} triggertype_e;

// define names for the Speed field of the general linedefs

typedef enum
{
  SpeedSlow,
  SpeedNormal,
  SpeedFast,
  SpeedTurbo,
} motionspeed_e;

// define names for the Target field of the general floor

typedef enum
{
  FtoHnF,
  FtoLnF,
  FtoNnF,
  FtoLnC,
  FtoC,
  FbyST,
  Fby24,
  Fby32,
} floortarget_e;

// define names for the Changer Type field of the general floor

typedef enum
{
  FNoChg,
  FChgZero,
  FChgTxt,
  FChgTyp,
} floorchange_e;

// define names for the Change Model field of the general floor

typedef enum
{
  FTriggerModel,
  FNumericModel,
} floormodel_t;

// define names for the Target field of the general ceiling

typedef enum
{
  CtoHnC,
  CtoLnC,
  CtoNnC,
  CtoHnF,
  CtoF,
  CbyST,
  Cby24,
  Cby32,
} ceilingtarget_e;

// define names for the Changer Type field of the general ceiling

typedef enum
{
  CNoChg,
  CChgZero,
  CChgTxt,
  CChgTyp,
} ceilingchange_e;

// define names for the Change Model field of the general ceiling

typedef enum
{
  CTriggerModel,
  CNumericModel,
} ceilingmodel_t;

// define names for the Target field of the general lift

typedef enum
{
  F2LnF,
  F2NnF,
  F2LnC,
  LnF2HnF,
} lifttarget_e;

// define names for the door Kind field of the general ceiling

typedef enum
{
  OdCDoor,
  ODoor,
  CdODoor,
  CDoor,
} doorkind_e;

// define names for the locked door Kind field of the general ceiling

typedef enum
{
  AnyKey,
  RCard,
  BCard,
  YCard,
  RSkull,
  BSkull,
  YSkull,
  AllKeys,
} keykind_e;

//////////////////////////////////////////////////////////////////
//
// enums for classes of linedef triggers
//
//////////////////////////////////////////////////////////////////

//jff 2/23/98 identify the special classes that can share sectors

typedef enum
{
  floor_special,
  ceiling_special,
  lighting_special,
} special_e;

//jff 3/15/98 pure texture/type change for better generalized support
typedef enum
{
  trigChangeOnly,
  numChangeOnly,
} change_e;

// p_plats

typedef enum
{
  up,
  down,
  waiting,
  in_stasis
} plat_e;

typedef enum
{
  perpetualRaise,
  downWaitUpStay,
  raiseAndChange,
  raiseToNearestAndChange,
  blazeDWUS,
  genLift,      //jff added to support generalized Plat types
  genPerpetual, 
  toggleUpDn,   //jff 3/14/98 added to support instant toggle type

} plattype_e;

// p_doors

typedef enum
{
  doorNormal, // haleyjd
  close30ThenOpen,
  doorClose, // haleyjd
  doorOpen,  // haleyjd
  raiseIn5Mins,
  blazeRaise,
  blazeOpen,
  blazeClose,

  //jff 02/05/98 add generalize door types
  genRaise,
  genBlazeRaise,
  genOpen,
  genBlazeOpen,
  genClose,
  genBlazeClose,
  genCdO,
  genBlazeCdO,
} vldoor_e;

// p_ceilng

typedef enum
{
  lowerToFloor,
  raiseToHighest,
  lowerToLowest,
  lowerToMaxFloor,
  lowerAndCrush,
  crushAndRaise,
  fastCrushAndRaise,
  silentCrushAndRaise,

  //jff 02/04/98 add types for generalized ceiling mover
  genCeiling,
  genCeilingChg,
  genCeilingChg0,
  genCeilingChgT,

  //jff 02/05/98 add types for generalized ceiling mover
  genCrusher,
  genSilentCrusher,

} ceiling_e;

// p_floor

typedef enum
{
  // lower floor to highest surrounding floor
  lowerFloor,
  
  // lower floor to lowest surrounding floor
  lowerFloorToLowest,
  
  // lower floor to highest surrounding floor VERY FAST
  turboLower,
  
  // raise floor to lowest surrounding CEILING
  raiseFloor,
  
  // raise floor to next highest surrounding floor
  raiseFloorToNearest,

  //jff 02/03/98 lower floor to next lowest neighbor
  lowerFloorToNearest,

  //jff 02/03/98 lower floor 24 absolute
  lowerFloor24,

  //jff 02/03/98 lower floor 32 absolute
  lowerFloor32Turbo,

  // raise floor to shortest height texture around it
  raiseToTexture,
  
  // lower floor to lowest surrounding floor
  //  and change floorpic
  lowerAndChange,

  raiseFloor24,

  //jff 02/03/98 raise floor 32 absolute
  raiseFloor32Turbo,

  raiseFloor24AndChange,
  raiseFloorCrush,

  // raise to next highest floor, turbo-speed
  raiseFloorTurbo,       
  donutRaise,
  raiseFloor512,

  //jff 02/04/98  add types for generalized floor mover
  genFloor,
  genFloorChg,
  genFloorChg0,
  genFloorChgT,

  //new types for stair builders
  buildStair,
  genBuildStair,
} floor_e;

typedef enum
{
  build8, // slowly build by 8
  turbo16 // quickly build by 16
    
} stair_e;

typedef enum
{
  elevateUp,
  elevateDown,
  elevateCurrent,
} elevator_e;

//////////////////////////////////////////////////////////////////
//
// general enums
//
//////////////////////////////////////////////////////////////////

// texture type enum
typedef enum
{
    top,
    middle,
    bottom

} bwhere_e;

// crush check returns
typedef enum
{
  ok,
  crushed,
  pastdest
} result_e;

//////////////////////////////////////////////////////////////////
//
// linedef and sector special data types
//
//////////////////////////////////////////////////////////////////

// p_switch

// switch animation structure type

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif

typedef PACKED_PREFIX struct
{
  char name1[9];
  char name2[9];
  short episode;
} PACKED_SUFFIX switchlist_t; //jff 3/23/98 pack to read from memory

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

typedef struct
{
  struct line_s *line;
  bwhere_e where;
  int   btexture;
  int   btimer;
  struct mobj_s *soundorg;
} button_t;

void P_StartButton(struct line_s *line, bwhere_e w, int texture, int time);

// p_lights

typedef struct
{
  thinker_t thinker;
  struct sector_s *sector;
  int count;
  int maxlight;
  int minlight;
} fireflicker_t;

typedef struct
{
  thinker_t thinker;
  struct sector_s *sector;
  int count;
  int maxlight;
  int minlight;
  int maxtime;
  int mintime;
} lightflash_t;

typedef struct
{
  thinker_t thinker;
  struct sector_s *sector;
  int count;
  int minlight;
  int maxlight;
  int darktime;
  int brighttime;
} strobe_t;

typedef struct
{
  thinker_t thinker;
  struct sector_s *sector;
  int minlight;
  int maxlight;
  int direction;

} glow_t;

// p_plats

typedef struct plat_s
{
  thinker_t thinker;
  struct sector_s *sector;
  fixed_t speed;
  fixed_t low;
  fixed_t high;
  int wait;
  int count;
  plat_e status;
  plat_e oldstatus;
  boolean crush;
  int tag;
  plattype_e type;
  struct platlist *list;   // killough
} plat_t;

// New limit-free plat structure -- killough

typedef struct platlist {
  plat_t *plat; 
  struct platlist *next,**prev;
} platlist_t;

// p_ceilng

typedef struct
{
  thinker_t thinker;
  vldoor_e type;
  struct sector_s *sector;
  fixed_t topheight;
  fixed_t speed;

  // 1 = up, 0 = waiting at top, -1 = down
  int direction;
  
  // tics to wait at the top
  int topwait;
  // (keep in case a door going down is reset)
  // when it reaches 0, start going down
  int topcountdown;
  
  //jff 1/31/98 keep track of line door is triggered by
  struct line_s *line;

  int lighttag; //killough 10/98: sector tag for gradual lighting effects
} vldoor_t;

// p_doors

typedef struct ceiling_s
{
  thinker_t thinker;
  ceiling_e type;
  struct sector_s *sector;
  fixed_t bottomheight;
  fixed_t topheight;
  fixed_t speed;
  fixed_t oldspeed;
  boolean crush;

  //jff 02/04/98 add these to support ceiling changers
  int newspecial;
  int oldspecial; //jff 3/14/98 add to fix bug in change transfers
  short texture;

  // 1 = up, 0 = waiting, -1 = down
  int direction;

  // ID
  int tag;                   
  int olddirection;
  struct ceilinglist *list;   // jff 2/22/98 copied from killough's plats
} ceiling_t;

typedef struct ceilinglist {
  ceiling_t *ceiling; 
  struct ceilinglist *next,**prev;
} ceilinglist_t;

// p_floor

typedef struct
{
  thinker_t thinker;
  floor_e type;
  boolean crush;
  struct sector_s *sector;
  int direction;
  int newspecial;
  int oldspecial;   //jff 3/14/98 add to fix bug in change transfers
  short texture;
  fixed_t floordestheight;
  fixed_t speed;
} floormove_t;

typedef struct
{
  thinker_t thinker;
  elevator_e type;
  struct sector_s *sector;
  int direction;
  fixed_t floordestheight;
  fixed_t ceilingdestheight;
  fixed_t speed;
} elevator_t;

// p_spec

// killough 3/7/98: Add generalized scroll effects

typedef struct {
  thinker_t thinker;   // Thinker structure for scrolling
  fixed_t dx, dy;      // (dx,dy) scroll speeds
  int affectee;        // Number of affected sidedef, sector, tag, or whatever
  int control;         // Control sector (-1 if none) used to control scrolling
  fixed_t last_height; // Last known height of control sector
  fixed_t vdx, vdy;    // Accumulated velocity if accelerative
  int accel;           // Whether it's accelerative
  enum
  {
    sc_side,
    sc_floor,
    sc_ceiling,
    sc_carry,
    sc_carry_ceiling,  // killough 4/11/98: carry objects hanging on ceilings
  } type;              // Type of scroll effect
} scroll_t;

// phares 3/12/98: added new model of friction for ice/sludge effects

typedef struct {
  thinker_t thinker;   // Thinker structure for friction
  int friction;        // friction value (E800 = normal)
  int movefactor;      // inertia factor when adding to momentum
  int affectee;        // Number of affected sector
} friction_t;

// phares 3/20/98: added new model of Pushers for push/pull effects

typedef struct {
  thinker_t thinker;   // Thinker structure for Pusher
  enum
  {
    p_push,
    p_pull,
    p_wind,
    p_current,
  } type;
  struct mobj_s *source;      // Point source if point pusher
  int x_mag;           // X Strength
  int y_mag;           // Y Strength
  int magnitude;       // Vector strength for point pusher
  int radius;          // Effective radius for point pusher
  int x;               // X of point source if point pusher
  int y;               // Y of point source if point pusher
  int affectee;        // Number of affected sector
} pusher_t;

//////////////////////////////////////////////////////////////////
//
// external data declarations
//
//////////////////////////////////////////////////////////////////

//
// End-level timer (-TIMER option)
//
extern  boolean levelTimer;
extern  int levelTimeCount;

// list of retriggerable buttons active
extern button_t buttonlist[MAXBUTTONS];

extern platlist_t *activeplats;        // killough 2/14/98

extern ceilinglist_t *activeceilings;  // jff 2/22/98

////////////////////////////////////////////////////////////////
//
// Linedef and sector special utility function prototypes
//
////////////////////////////////////////////////////////////////

int twoSided(int sector, int line);

struct sector_s *getSector(int currentSector, int line, int side);

struct side_s *getSide(int currentSector, int line, int side);

fixed_t P_FindLowestFloorSurrounding(struct sector_s *sec);

fixed_t P_FindHighestFloorSurrounding(struct sector_s *sec);

fixed_t P_FindNextHighestFloor(struct sector_s *sec, int currentheight);

fixed_t P_FindNextLowestFloor(struct sector_s *sec, int currentheight);

fixed_t P_FindLowestCeilingSurrounding(struct sector_s *sec); // jff 2/04/98

fixed_t P_FindHighestCeilingSurrounding(struct sector_s *sec); // jff 2/04/98

fixed_t P_FindNextLowestCeiling(struct sector_s *sec, int currentheight); // jff 2/04/98

fixed_t P_FindNextHighestCeiling(struct sector_s *sec, int currentheight); // jff 2/04/98

fixed_t P_FindShortestTextureAround(int secnum); // jff 2/04/98

fixed_t P_FindShortestUpperAround(int secnum); // jff 2/04/98

struct sector_s *P_FindModelFloorSector(fixed_t floordestheight, int secnum); //jff 02/04/98

struct sector_s *P_FindModelCeilingSector(fixed_t ceildestheight, int secnum); //jff 02/04/98 

int P_FindSectorFromLineTag(const struct line_s *line, int start); // killough 4/17/98

int P_FindLineFromLineTag(const struct line_s *line, int start);   // killough 4/17/98

int P_FindMinSurroundingLight(struct sector_s *sector, int max);

struct sector_s *getNextSector(struct line_s *line, struct sector_s *sec);

int P_CheckTag(struct line_s *line); // jff 2/27/98

boolean P_CanUnlockGenDoor(struct line_s *line, struct player_s *player);

int P_SectorActive(special_e t, struct sector_s *s);

boolean P_IsDeathExit(struct sector_s *sec);

boolean P_IsSecret(struct sector_s *sec);

boolean P_WasSecret(struct sector_s *sec);

void P_ChangeSwitchTexture(struct line_s *line, int useAgain);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special action function prototypes
//
////////////////////////////////////////////////////////////////

// p_lights

void T_LightFlash(lightflash_t *flash);

void T_StrobeFlash(strobe_t *flash);

void T_Glow(glow_t *g);

void T_FireFlicker(fireflicker_t *flick);  // killough 10/4/98

// p_plats

void T_PlatRaise(plat_t *plat);

// p_doors

void T_VerticalDoor(vldoor_t *door);

// p_ceilng

void T_MoveCeiling(ceiling_t *ceiling);

// p_floor

result_e T_MovePlane(struct sector_s *sector, fixed_t speed, fixed_t dest,
                     boolean crush, int floorOrCeiling, int direction);

void T_MoveFloor(floormove_t *floor);

void T_MoveElevator(elevator_t *elevator);

// p_spec

void T_Scroll(scroll_t *);      // killough 3/7/98: scroll effect thinker

void T_Friction(friction_t *);  // phares 3/12/98: friction thinker

void T_Pusher(pusher_t *);      // phares 3/20/98: Push thinker

////////////////////////////////////////////////////////////////
//
// Linedef and sector special handler prototypes
//
////////////////////////////////////////////////////////////////

// p_telept

int EV_Teleport(struct line_s *line, int side, struct mobj_s *thing);

// killough 2/14/98: Add silent teleporter
int EV_SilentTeleport(struct line_s *line, int side, struct mobj_s *thing);

// killough 1/31/98: Add silent line teleporter
int EV_SilentLineTeleport(struct line_s *line, int side, 
                          struct mobj_s *thing, boolean reverse);

// p_floor

int EV_DoElevator(struct line_s *line, elevator_e elevtype);

int EV_BuildStairs(struct line_s *line, stair_e type);

int EV_DoFloor(struct line_s *line, floor_e floortype);

// p_ceilng

int EV_DoCeiling(struct line_s *line, ceiling_e type);

int EV_CeilingCrushStop(struct line_s *line);

// p_doors

int EV_VerticalDoor(struct line_s *line, struct mobj_s *thing);

int EV_DoDoor(struct line_s *line, vldoor_e type);

int EV_DoLockedDoor(struct line_s *line, vldoor_e type, struct mobj_s *thing);

// p_lights

int EV_StartLightStrobing(struct line_s *line);

int EV_TurnTagLightsOff(struct line_s *line);

int EV_LightTurnOn(struct line_s *line, int bright);

int EV_LightTurnOnPartway(struct line_s *line, fixed_t level);  // killough 10/10/98

// p_floor

int EV_DoChange(struct line_s *line, change_e changetype);

int EV_DoDonut(struct line_s *line);

// p_plats

int EV_DoPlat(struct line_s *line, plattype_e type, int amount);

int EV_StopPlat(struct line_s *line);

// p_genlin

int EV_DoGenFloor(struct line_s *line);

int EV_DoGenCeiling(struct line_s *line);

int EV_DoGenLift(struct line_s *line);

int EV_DoGenStairs(struct line_s *line);

int EV_DoGenCrusher(struct line_s *line);

int EV_DoGenDoor(struct line_s *line);

int EV_DoGenLockedDoor(struct line_s *line);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special thinker spawning
//
////////////////////////////////////////////////////////////////

// at game start
void P_InitPicAnims(void);

void P_InitSwitchList(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);

// when needed
boolean P_UseSpecialLine(struct mobj_s *thing, struct line_s *line, int side,
                         boolean bossaction);

void P_ShootSpecialLine(struct mobj_s *thing, struct line_s *line);

void P_CrossSpecialLine(struct line_s *, int side, struct mobj_s *thing,
                        boolean bossaction); // killough 11/98

extern int disable_nuke;  // killough 12/98: nukage disabling cheat

void P_PlayerInSpecialSector(struct player_s *player);

// p_lights

void P_SpawnFireFlicker(struct sector_s *sector);

void P_SpawnLightFlash(struct sector_s *sector);

void P_SpawnStrobeFlash(struct sector_s *sector, int fastOrSlow, int inSync);

void P_SpawnGlowingLight(struct sector_s *sector);

// p_plats

void P_AddActivePlat(plat_t *plat);

void P_RemoveActivePlat(plat_t *plat);

void P_RemoveAllActivePlats(void);    // killough

void P_ActivateInStasis(int tag);

// p_doors

void P_SpawnDoorCloseIn30(struct sector_s *sec);

void P_SpawnDoorRaiseIn5Mins(struct sector_s *sec, int secnum);

// p_ceilng

void P_RemoveActiveCeiling(ceiling_t *ceiling);  //jff 2/22/98

void P_RemoveAllActiveCeilings(void);                //jff 2/22/98

void P_AddActiveCeiling(ceiling_t *c);

int P_ActivateInStasisCeiling(struct line_s *line); 

struct mobj_s *P_GetPushThing(int);                                // phares 3/23/98

void P_HitFloor (struct mobj_s *mo, int oof); // [FG] play sound when hitting animated floor

#endif

//----------------------------------------------------------------------------
//
// $Log: p_spec.h,v $
// Revision 1.30  1998/05/04  02:22:23  jim
// formatted p_specs, moved a coupla routines to p_floor
//
// Revision 1.28  1998/04/17  10:25:04  killough
// Add P_FindLineFromLineTag()
//
// Revision 1.27  1998/04/14  18:49:50  jim
// Added monster only and reverse teleports
//
// Revision 1.26  1998/04/12  02:05:54  killough
// Add ceiling light setting, start ceiling carriers
//
// Revision 1.25  1998/04/05  13:54:03  jim
// fixed switch change on second activation
//
// Revision 1.24  1998/03/31  16:52:09  jim
// Fixed uninited type field in stair builders
//
// Revision 1.23  1998/03/23  18:38:39  jim
// Switch and animation tables now lumps
//
// Revision 1.22  1998/03/23  15:24:47  phares
// Changed pushers to linedef control
//
// Revision 1.21  1998/03/20  14:24:48  jim
// Gen ceiling target now shortest UPPER texture
//
// Revision 1.20  1998/03/20  00:30:27  phares
// Changed friction to linedef control
//
// Revision 1.19  1998/03/19  16:49:00  jim
// change sector bits to combine ice and sludge
//
// Revision 1.18  1998/03/16  12:39:08  killough
// Add accelerative scrollers
//
// Revision 1.17  1998/03/15  14:39:39  jim
// added pure texture change linedefs & generalized sector types
//
// Revision 1.16  1998/03/14  17:18:56  jim
// Added instant toggle floor type
//
// Revision 1.15  1998/03/09  07:24:40  killough
// Add scroll_t for generalized scrollers
//
// Revision 1.14  1998/03/02  12:11:35  killough
// Add scroll_effect_offset declaration
//
// Revision 1.13  1998/02/27  19:20:42  jim
// Fixed 0 tag trigger activation
//
// Revision 1.12  1998/02/23  23:47:15  jim
// Compatibility flagged multiple thinker support
//
// Revision 1.11  1998/02/23  00:42:12  jim
// Implemented elevators
//
// Revision 1.9  1998/02/17  06:20:32  killough
// Add prototypes, redefine MAXBUTTONS
//
// Revision 1.8  1998/02/13  03:28:17  jim
// Fixed W1,G1 linedefs clearing untriggered special, cosmetic changes
//
// Revision 1.7  1998/02/09  03:09:37  killough
// Remove limit on switches
//
// Revision 1.6  1998/02/08  05:35:48  jim
// Added generalized linedef types
//
// Revision 1.4  1998/02/02  13:43:55  killough
// Add silent teleporter
//
// Revision 1.3  1998/01/30  14:44:03  jim
// Added gun exits, right scrolling walls and ceiling mover specials
//
// Revision 1.2  1998/01/26  19:27:29  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:01  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------

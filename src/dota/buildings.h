#ifndef __DOTA_BUILDINGS_H__
#define __DOTA_BUILDINGS_H__

#include "dota/consts.h"
#include "base/string.h"

#define BUILDING_TOWER      0
#define BUILDING_MELEE      1
#define BUILDING_RANGED     2
#define BUILDING_THRONE     3

struct DotaBuilding
{
  double x;
  double y;
  String icon;
};
enum {
  BUILDING_SENTINEL_TOWER_TOP1,   // e00R
  BUILDING_SENTINEL_TOWER_TOP2,   // e011
  BUILDING_SENTINEL_TOWER_TOP3,   // e00S
  BUILDING_SENTINEL_TOWER_MID1,   // e00R
  BUILDING_SENTINEL_TOWER_MID2,   // e011
  BUILDING_SENTINEL_TOWER_MID3,   // e00S
  BUILDING_SENTINEL_TOWER_BOT1,   // e00R
  BUILDING_SENTINEL_TOWER_BOT2,   // e011
  BUILDING_SENTINEL_TOWER_BOT3,   // e00S
  BUILDING_SENTINEL_MELEE_TOP,    // eaom
  BUILDING_SENTINEL_MELEE_MID,    // eaom
  BUILDING_SENTINEL_MELEE_BOT,    // eaom
  BUILDING_SENTINEL_RANGED_TOP,   // eaoe
  BUILDING_SENTINEL_RANGED_MID,   // eaoe
  BUILDING_SENTINEL_RANGED_BOT,   // eaoe
  BUILDING_SENTINEL_TOWER1,       // e019
  BUILDING_SENTINEL_TOWER2,       // e019
  BUILDING_WORLD_TREE,            // etol -5632.0,-6144.0

  BUILDINGS_SCOURGE,
  BUILDING_SCOURGE_TOWER_TOP1 = BUILDINGS_SCOURGE,    // u00M
  BUILDING_SCOURGE_TOWER_TOP2,    // u00D
  BUILDING_SCOURGE_TOWER_TOP3,    // u00N
  BUILDING_SCOURGE_TOWER_MID1,    // u00M
  BUILDING_SCOURGE_TOWER_MID2,    // u00D
  BUILDING_SCOURGE_TOWER_MID3,    // u00N
  BUILDING_SCOURGE_TOWER_BOT1,    // u00M
  BUILDING_SCOURGE_TOWER_BOT2,    // u00D
  BUILDING_SCOURGE_TOWER_BOT3,    // u00N
  BUILDING_SCOURGE_MELEE_TOP,     // usep
  BUILDING_SCOURGE_MELEE_MID,     // usep
  BUILDING_SCOURGE_MELEE_BOT,     // usep
  BUILDING_SCOURGE_RANGED_TOP,    // utod
  BUILDING_SCOURGE_RANGED_MID,    // utod
  BUILDING_SCOURGE_RANGED_BOT,    // utod
  BUILDING_SCOURGE_TOWER1,        // u00T
  BUILDING_SCOURGE_TOWER2,        // u00T
  BUILDING_FROZEN_THRONE,         // unpl

  NUM_BUILDINGS
};
DotaBuilding* getBuildings();
int getBuildingId(int side, int type, int lane, int level);

#endif // __DOTA_BUILDINGS_H__

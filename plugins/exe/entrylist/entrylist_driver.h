/**
 * @ingroup  exe_plugin
 * @file     entrylist_driver.h
 * @author   Benjamin Gerard <ben@sashipa.com>
 * @date     2002/10/23
 * @brief    entry-list lua extension plugin
 * 
 * $Id: entrylist_driver.h,v 1.2 2002-10-24 18:57:07 benjihan Exp $
 */

#ifndef _ENTRYLIST_DRIVER_H_
#define _ENTRYLIST_DRIVER_H_

#include <stdio.h>
#include "lua.h"
#include "any_driver.h"
#include "allocator.h"
#include "entrylist.h"

/**< Entrylist user tag. */
extern int entrylist_tag;
/**< Holds all entrylist. */
extern allocator_t * lists;
/**< Holds standard entries. */
extern allocator_t * entries;

int lua_entrylist_init(lua_State * L);

/** Entrylist driver name. */
#define DRIVER_NAME "entrylist"

#define EL_FUNCTION_DECLARE(name) int lua_entrylist_##name(lua_State * L)

#define EL_FUNCTION_START(name) \
  int lua_entrylist_##name(lua_State * L) \
  { \
    el_list_t * el; \
    if (lua_tag(L, 1) != entrylist_tag) { \
      printf("el_" #name " : first parameter is not an entry-list\n"); \
      return 0; \
    } \
    if (el = lua_touserdata(L, 1), !el) { \
      printf("el_" #name " : Null pointer.\n"); \
      return 0; \
    }

#define EL_FUNCTION_END() }

#endif /* #define _ENTRYLIST_DRIVER_H_ */
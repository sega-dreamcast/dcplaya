#ifndef _DRIVER_LIST_H_
#define _DRIVER_LIST_H_

#include "any_driver.h"
#include "inp_driver.h"

/** Any driver list. */
typedef struct
{
  int n;                  /**< Number of driver in list */
  any_driver_t * drivers; /**< First entry */
} driver_list_t;

extern driver_list_t inp_drivers;
extern driver_list_t obj_drivers;
extern driver_list_t vis_drivers;
extern driver_list_t exe_drivers;

/** Init all driver list */
int driver_list_init_all();

/** Shutddown all driver list */
void driver_list_shutdown_all(void);

int driver_list_init(driver_list_t * dl);
void driver_list_shutdown(driver_list_t * dl);

int driver_list_register(driver_list_t * dl, any_driver_t * driver);
int driver_list_unregister(driver_list_t * dl, any_driver_t * driver);
int driver_register(any_driver_t * driver);

any_driver_t * driver_list_search(driver_list_t * dl, const char *name);


/* input driver function */
inp_driver_t * inp_driver_list_search_by_extension(const char *ext);
inp_driver_t * inp_driver_list_search_by_id(int id);

#endif
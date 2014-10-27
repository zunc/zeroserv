/* 
 * File:   route_map.h
 * Author: khoai
 *
 * Created on October 17, 2014, 10:25 AM
 */

#ifndef ROUTE_MAP_H
#define	ROUTE_MAP_H

#include "type/route.h"

void route_register(struct route *r);
void route_unregister(struct route *r);
struct route* route_get_by_name(const char* name);
struct route* route_get_by_dst(const char* dst);

int route_register_config(const char *name, const char *src, const char *remote);

#endif	/* ROUTE_MAP_H */


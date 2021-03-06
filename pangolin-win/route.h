#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include "route.h"
using namespace std;

class RouteItem {
public:
	uint32_t addr, dst, mask, next;
	int ifIndex;

	RouteItem();
	RouteItem(uint32_t addr, uint32_t dst, uint32_t mask, uint32_t next, int if_index);
};

class Route {
public:
	Route();

	vector<RouteItem> routes;

public:
	RouteItem* getRoute(uint32_t dst);
	string toString();
};
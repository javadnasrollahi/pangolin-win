#include "route.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <cstdio>
#include <iostream>
#include "../ethernet/util.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

RouteItem::RouteItem() {
	dst = 0; mask = 0; next = 0; ifIndex = 0;
}

RouteItem::RouteItem(uint32_t dst, uint32_t mask, uint32_t next, int ifIndex){
	this->dst = dst;
	this->mask = mask;
	this->next = next;
	this->ifIndex = ifIndex;
}

Route::Route() {
	PMIB_IPFORWARDTABLE pTable;
	DWORD dwSize = 0, dwRetVal = 0;
	pTable = (MIB_IPFORWARDTABLE*)malloc(sizeof(MIB_IPFORWARDTABLE));
	if (pTable == NULL) {
		cout << "error allocating memory" << endl;
		exit(-1);
	}

	if (GetIpForwardTable(pTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
		free(pTable);
		pTable = (MIB_IPFORWARDTABLE*)malloc(dwSize);
		if (pTable == NULL) {
			cout << "error allocating memory" << endl;
			exit(-1);
		}
	}

	if ((dwRetVal = GetIpForwardTable(pTable, &dwSize, 0)) == NO_ERROR) {
		int n = pTable->dwNumEntries;
		for (int i = 0; i < n; i++) {
			RouteItem it;
			it.dst = revIp((uint32_t)pTable->table[i].dwForwardDest);
			it.mask = revIp((uint32_t)pTable->table[i].dwForwardMask);
			it.next = revIp((uint32_t)pTable->table[i].dwForwardNextHop);
			it.ifIndex = pTable->table[i].dwForwardIfIndex;
			routes.push_back(it);
		}
		free(pTable);
	}
	else {
		free(pTable);
		cout << "get route table error" << endl;
		exit(-1);
	}
}

RouteItem* Route::getRoute(uint32_t dst) {
	int ln = routes.size();
	for (int i = ln - 1; i >= 0; i--) {
		if ((dst & routes[i].mask) == routes[i].dst) {
			return &routes[i];
		}
	}
	return NULL;
}

string Route::toString() {
	char fmt[] = "dst: %s, mask: %s, next: %s, index: %d\n";
	char buf[1024];
	string res = "";
	for (int i = 0; i < routes.size(); i++) {
		sprintf_s(buf, 1024, fmt, 
			ip2str(routes[i].dst).c_str(), 
			ip2str(routes[i].mask).c_str(), 
			ip2str(routes[i].next).c_str(), 
			routes[i].ifIndex);
		res += buf;
	}
	return res;
}
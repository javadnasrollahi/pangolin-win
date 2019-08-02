#include "tun.h"
#include "../ethernet/util.h"
#include "../ethernet/const.h"
#include <tchar.h>
#include <thread>
#include <iostream>


Tun::Tun(string server) {
	handle = NULL;
	this->server = server;
}

Tun::~Tun(){}

vector<uint8_t> Tun::read() {
	vector<uint8_t> res;
	UINT rn = 0;
	WINDIVERT_ADDRESS addr;
	if(!WinDivertRecv(handle, buf, BUFFSIZE, &rn, &addr)) {
		return res;
	}
	
	for (int i = 0; i < rn; i++) {
		res.push_back(buf[i]);
	}
	return res;
}

bool Tun::write(vector<uint8_t>& data) {
	for (int i = 0; i < data.size(); i++) {
		buf[i] = data[i];
	}
	Frame frame;
	int rn = frame.read(3, buf, BUFFSIZE);
	if (rn <= 0) return false;
	rn = frame.write(3, buf, BUFFSIZE);

	UINT wn = 0;
	WINDIVERT_ADDRESS addr;
	addr.Layer = WINDIVERT_LAYER_NETWORK;
	addr.Impostor = 1;
	addr.Loopback = 0;
	addr.Outbound = 0;
	addr.IPChecksum = 1;
	addr.TCPChecksum = 1;
	addr.UDPChecksum = 1;
	addr.Network.IfIdx = 14;
	addr.Network.SubIfIdx = 0;
	WinDivertSend(handle, buf, rn, &wn, &addr);

	return true;
}

bool Tun::start() {
	char fmt[] = "ip.SrcAddr != %s and ip.DstAddr != %s and !loopback";
	char filter[1024];
	sprintf_s(filter, 1024, fmt, server.c_str(), server.c_str());
	handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
	return !(handle == INVALID_HANDLE_VALUE);
}

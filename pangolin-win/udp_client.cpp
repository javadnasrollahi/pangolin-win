#include <WinSock2.h>
#include <WS2tcpip.h>
#include "udp_client.h"
#include "tun.h"
#include "../ethernet/util.h"
#include <iostream>
using namespace std;

Udp_client::Udp_client(Config *config, Tun* tun) {
	this->config = config;
	this->tun = tun;
}

Udp_client::~Udp_client() {
}


bool Udp_client::start() {
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0) {
		return false;
	}

	struct sockaddr_in server_info;
	int len = sizeof(server_info);
	memset(&server_info, 0, len);
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(config->server_port);
	inet_pton(AF_INET, config->server_ip.c_str(), (void*)& server_info.sin_addr.S_un.S_addr);

	SOCKET sk = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sk == SOCKET_ERROR) {
		return false;
	}

	u_long mode = 1;
	ioctlsocket(sk, FIONBIO, &mode);
	char buf[2500];

	while (true) {
		vector<uint8_t> data = tun->read();
		if (data.size() > 0 && data.size() < 2500) {
			for (int i = 0; i < data.size(); i++) {
				buf[i] = data[i];
			}
			Frame frame;
			if (frame.read(3, (uint8_t*)buf, 2500) <= 0) continue;
			int wn = frame.write(3, (uint8_t*)buf, 2500);

			sendto(sk, buf, wn, 0, (sockaddr*)& server_info, sizeof(sockaddr));
			//cout << "send  " << data.size() << endl;
		}

		int rl = recvfrom(sk, buf, 2500, 0, (sockaddr*)& server_info, &len);
		if (rl > 0){
			vector<uint8_t> data;
			for (int i = 0; i < rl; i++) {
				data.push_back(buf[i]);
				printf("%X ", data[i]);
			}
			Frame frame;
			frame.read(3, (uint8_t*)buf, rl);
			cout << frame.ipv4.to_string() << endl;

			cout << endl;
			cout << "recv  " << data.size() << endl;
			tun->write(data);
		}
	}
}
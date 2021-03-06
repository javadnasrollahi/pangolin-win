#include "Ptcp_client.h"

Ptcp_client::Ptcp_client(Config* config, Ptcp* ptcp, Tun* tun) {
	this->config = config;
	this->ptcp = ptcp;
	this->tun = tun;
	this->encrypt = new Encrypt(config->token);
}

Ptcp_client::~Ptcp_client() {
}

bool Ptcp_client::start() {
	if (!tun->start()) return false;
	cout << "tun start done" << endl;
	if (!ptcp->start()) return false;
	cout << "ptcp start done" << endl;
	if (!ptcp->dial_timeout(30)) return false;
	cout << "dial done" << endl;
	if (!login()) return false;
	cout << "login done" << endl;

	cout << "ptcp client started" << endl;
	thread send_thread(&Ptcp_client::send, this);
	thread recv_thread(&Ptcp_client::recv, this);
	send_thread.join();
	recv_thread.join();
}

void Ptcp_client::send() {
	while (true) {
		vector<uint8_t> data = tun->read();
		if (data.size() > 0 && data.size() < BUFFSIZE) {
			for (int i = 0; i < data.size(); i++) {
				send_buf[i] = data[i];
			}
			Frame frame;
			if (frame.read(3, (uint8_t*)send_buf, BUFFSIZE) <= 0) continue;
			int wn = frame.write(3, (uint8_t*)send_buf, BUFFSIZE);
			string datas = "";
			for (int i = 0; i < wn; i++) {
				datas.push_back((uint8_t)send_buf[i]);
			}
			string endata = encrypt->encrypt(datas);

			vector<uint8_t> data;
			data.push_back(Ptcp::PTCP_PACKETTYPE_DATA);
			for (int i = 0; i < endata.size(); i++) {
				data.push_back((uint8_t)endata[i]);
			}
			ptcp->send(data);
		}
	}
}

void Ptcp_client::recv() {
	while (true) {
		vector<uint8_t> data = ptcp->recv();
		if (data.size() > 1 && data[0] == Ptcp::PTCP_PACKETTYPE_DATA) {
			
			string endata;
			for (int i = 1; i < data.size(); i++) {
				endata.push_back(data[i]);
			}

			vector<uint8_t> data_send;
			string datas = encrypt->decrypt(endata);
			for (int i = 0; i < datas.size(); i++) {
				data_send.push_back(datas[i]);
			}
			tun->write(data_send);
		}
	}
}

bool login_check(vector<uint8_t>& data) {
	if (data.size() <= 1) return false;
	if (data[0] == Ptcp::PTCP_PACKETTYPE_LOGIN || data[1] == Ptcp::PTCP_LOGINMSG_SUCCESS) return true;
	return false;
}

bool Ptcp_client::login() {
	vector<uint8_t> data;
	data.push_back(Ptcp::PTCP_PACKETTYPE_LOGIN);
	for (uint8_t c : config->token) {
		data.push_back(c);
	}

	int n = ptcp->send_until_timeout(data, 30, login_check);
	if (n < 0) return false;
	return true;
}
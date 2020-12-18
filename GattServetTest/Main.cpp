#include "Main.h"

// g++ Main.cpp -o test -I/home/pi/bleconfd/build/deps/src/bluez -lbluetooth /home/pi/bleconfd/build/deps/src/bluez/src/.libs/libshared-mainloop.a /home/pi/bleconfd/build/deps/src/bluez/lib/.libs/uuid.o








int main() {



	/* gatt server init */

	int m_listen_fd;
	m_listen_fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

	if (m_listen_fd < 0)
		cout << "failed to create bluetooth socket";

	bdaddr_t src_addr = { 0 }; // BDADDR_ANY

	sockaddr_l2 srcaddr;
	memset(&srcaddr, 0, sizeof(srcaddr));
	srcaddr.l2_family = AF_BLUETOOTH;
	srcaddr.l2_cid = htobs(4);
	srcaddr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
	bacpy(&srcaddr.l2_bdaddr, &src_addr);

	int ret = bind(m_listen_fd, reinterpret_cast<sockaddr *>(&srcaddr), sizeof(srcaddr));
	if (ret < 0)
		cout << "failed to bind bluetooth socket";

	bt_security btsec = { 0, 0 };
	btsec.level = BT_SECURITY_LOW;
	ret = setsockopt(m_listen_fd, SOL_BLUETOOTH, BT_SECURITY, &btsec, sizeof(btsec));
	if (ret < 0)
		cout << "failed to set security on bluetooth socket";

	ret = listen(m_listen_fd, 2);
	if (ret < 0)
		cout << "failed to listen on bluetooth socket";

	/* gatt server accept */

	mainloop_init();

	sockaddr_l2 peer_addr;
	memset(&peer_addr, 0, sizeof(peer_addr));

	printf("waiting for incoming BLE connections");

	socklen_t n = sizeof(peer_addr);
	int soc = ::accept(m_listen_fd, reinterpret_cast<sockaddr *>(&peer_addr), &n);
	if (soc < 0)
		cout << "failed to accept incoming connection on bluetooth socket";

	char remote_address[64] = { 0 };
	ba2str(&peer_addr.l2_bdaddr, remote_address);

	cout << "accepted remote connection from: %s" << remote_address;

	/* gatt client init */

	bt_att* m_att = bt_att_new(soc, 0);
	if (!m_att)
	{
		cout << "failed to create new att" << errno;
	}

	bt_att_set_close_on_unref(m_att, true);
	bt_att_register_disconnect(m_att, &onClientDisconnected, nullptr, nullptr);
	gatt_db* m_db = gatt_db_new();
	if (!m_db)
	{
		cout << "failed to create gatt database";
	}

	int m_mtu = 16;

	bt_gatt_server* m_server = bt_gatt_server_new(m_db, m_att, m_mtu, 0);
	if (!m_server)
	{
		cout << "failed to create gatt server" << errno;
	}

	if (true)
	{
		//bt_att_set_debug(m_att, ATT_debugCallback, this, nullptr);
		//bt_gatt_server_set_debug(m_server, GATT_debugCallback, this, nullptr);
	}

	int m_timeout_id = mainloop_add_timeout(1000, &onTimeout, nullptr, nullptr);


	/* GattClient::buildGattDatabase */

	/* buildGapService */
	buildGapService(m_db);

	/* buildGattService */
	buildGattService(m_db);




	return 0;
}
#include "Main.h"

// g++ Main.cpp -o test -I/home/pi/bleconfd/build/deps/src/bluez -lbluetooth /home/pi/bleconfd/build/deps/src/bluez/src/.libs/libshared-mainloop.a /home/pi/bleconfd/build/deps/src/bluez/lib/.libs/uuid.o
// g++ beacon.cc -c -lbluetooth
// g++ util.cc -c

// g++ Main.cpp -o test -I/home/pi/bleconfd/build/deps/src/bluez -lbluetooth /home/pi/bleconfd/build/deps/src/bluez/src/.libs/libshared-mainloop.a /home/pi/bleconfd/build/deps/src/bluez/lib/.libs/uuid.o beacon.o util.o

#include <unistd.h>
#include <thread>
#include <sys/time.h>


using namespace std;


void send_notifications()
{

	usleep(10000000);
	int count = 0;

	uint8_t data[256] = { 0 };
	//const char *p = data;
	int tempsec = 0;
	timeval tv;
	gettimeofday(&tv, 0);
	tempsec = tv.tv_sec;

	while (1) {


		bool send_success = bt_gatt_server_send_notification(m_server,
			tomo_notify_handle,
			data,
			256);//notify_len)

		//cout << bt_gatt_server_get_write_queue_length(m_server) << endl;
		
		while(bt_gatt_server_get_write_queue_length(m_server) > 10)
			usleep(1000);

		data[15]++;
		data[255]++;
		data[220]++;
		count++;
		gettimeofday(&tv, 0);
		
		if (tempsec < tv.tv_sec) {

			cout << "Notification data: " << count * 20 / 1024 << " kb at " << tempsec << endl;
			tempsec = tv.tv_sec;
			data[9]++;
		}

	}

	
}



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

	startBeacon("XPI-SETUP", 0);

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

	int m_mtu = 256;

	//bt_gatt_server* m_server = bt_gatt_server_new(m_db, m_att, m_mtu, 0);
	m_server = bt_gatt_server_new(m_db, m_att, m_mtu, 0); // 這個是bluez 5.50版用的
	//m_server = bt_gatt_server_new(m_db, m_att, m_mtu);		// 這個是bluez 5.43版用的
	if (!m_server)
	{
		cout << "failed to create gatt server" << errno;
	}

	//do{
	//	m_server = bt_gatt_server_new(m_db, m_att, m_mtu);		// 這個是bluez 5.43版用的
	//	if (!m_server)
	//	{
	//		cout << "failed to create gatt server" << m_mtu << " : " << errno;
	//	}
	//
	//	m_mtu /= 2;
	//
	//} while (!m_server);

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

	//buildRpcService(m_db);

	buildTomofunService(m_db);

	/* GattClient::run */

	thread mThread(send_notifications);

	printf("before mainloop\n");
	printf("before mainloop\n");
	printf("before mainloop\n");

	mainloop_run();

	printf("after mainloop\n");
	printf("after mainloop\n");
	printf("after mainloop\n");


	return 0;
}

void GattClient_onGapRead(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp)
{
	//GattClient* clnt = reinterpret_cast<GattClient *>(argp);
	//clnt->onGapRead(attr, id, offset, opcode, att);

	printf("onGapRead %04x\n", id);

	uint8_t error = 0;
	size_t len = strlen(ServerName);
	if (offset > len)
	{
		error = BT_ATT_ERROR_INVALID_OFFSET;
	}
	else
	{
		error = 0;
		len -= offset;
		uint8_t const* value = nullptr;
		if (len)
		{
			value = reinterpret_cast<uint8_t const *>(ServerName + offset);
		}
		gatt_db_attribute_read_result(attr, id, error, value, len);
	}
}

void GattClient_onGapWrite(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t const* data, size_t len, uint8_t opcode, bt_att* att, void* argp)
{
	printf("onGapWrite\n");

	uint8_t error = 0;
	if ((offset + len) == 0)
	{
		memset(ServerName, 0, sizeof(ServerName));
	}
	else if (offset > sizeof(ServerName))
	{
		error = BT_ATT_ERROR_INVALID_OFFSET;
	}
	else
	{
		memcpy(ServerName + offset, data, len);
	}

	gatt_db_attribute_write_result(attr, id, error);
}

void GattClient_onServiceChanged(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp)
{
	printf("onServiceChanged\n");
	gatt_db_attribute_read_result(attr, id, 0, nullptr, 0);
}

void GattClient_onServiceChangedRead(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp)
{
	printf("onServiceChangedRead\n");

	uint8_t value[2]{ 0x00, 0x00 };
	if (m_service_change_enabled)
		value[0] = 0x02;
	gatt_db_attribute_read_result(attr, id, 0, value, sizeof(value));
}

void GattClient_onServiceChangedWrite(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t const* value, size_t len, uint8_t opcode, bt_att* att, void* argp)
{
	printf("onServiceChangeWrite\n");

	uint8_t ecode = 0;
	if (!value || (len != 2))
		ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;

	if (!ecode && offset)
		ecode = BT_ATT_ERROR_INVALID_OFFSET;

	if (!ecode)
	{
		if (value[0] == 0x00)
			m_service_change_enabled = false;
		else if (value[0] == 0x02)
			m_service_change_enabled = true;
		else
			ecode = 0x80;
	}

	gatt_db_attribute_write_result(attr, id, ecode);
}

void GattClient_onGapExtendedPropertiesRead(gatt_db_attribute *attr, uint32_t id,
	uint16_t offset, uint8_t opcode, bt_att* att, void* argp)
{
	uint8_t value[2];
	value[0] = BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE;
	value[1] = 0;
	gatt_db_attribute_read_result(attr, id, 0, value, sizeof(value));
}

//void GattClient_onClientDisconnected(int err, void* argp)
//{
//	GattClient* clnt = reinterpret_cast<GattClient *>(argp);
//	clnt->onClientDisconnected(err);
//}

void GattClient_onEPollRead(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp)
{
	uint32_t value = m_outgoing_queue.size();

	printf("onEPollRead(offset=%d, opcode=%d)\n", offset, opcode);

	value = htonl(value);
	gatt_db_attribute_read_result(attr, id, 0, reinterpret_cast<uint8_t const *>(&value),
		sizeof(value));
}

void GattClient_onDataChannelIn(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t const* data, size_t len, uint8_t opcode, bt_att* att, void* argp)
{
	printf("onDataChannelIn(offset=%d, len=%zd)\n", offset, len);

	// TODO: should this use memory_stream?
	for (size_t i = 0; i < len; ++i)
	{
		char c = static_cast<char>(data[i + offset]);
		m_incoming_buff.push_back(c);

		if (c == kRecordDelimiter)
		{
			// 這邊晚點補，要用到rpc handler
			//if (!m_data_handler)
			//{
			//	// TODO:
			//	printf("no data handler registered\n");
			//}
			//else
			//{
			//	m_incoming_buff.push_back('\0');
			//	m_data_handler(&m_incoming_buff[0], m_incoming_buff.size());
			//}
			//m_incoming_buff.clear();
		}
	}

	gatt_db_attribute_write_result(attr, id, 0);
}

void GattClient_onDataChannelOut(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp)
{
	printf("onDataChannelOut(id=%d, offset=%u, opcode=%d)\n",
		id, offset, opcode);

	static int32_t const kBufferSize = 256;
	static uint8_t buff[kBufferSize];

	int n = 0;

	if (offset == 0)
	{
		memset(buff, 0, sizeof(buff));
		n = m_outgoing_queue.get_line((char *)buff, kBufferSize);
	}
	else
	{
		int bytesToWrite = 0;
		while ((bytesToWrite + offset) < kBufferSize && (buff[offset + bytesToWrite] != '\0'))
			bytesToWrite++;

		n = bytesToWrite;
		printf("bytesToWrite:%d offset:%d n:%d\n", bytesToWrite, offset, n);
	}

	uint8_t const* value = nullptr;
	if (n > 0)
		value = buff + offset;
	else
		buff[0] = '\0';

	printf("write:%.*s\n", n, (char *) value);
	gatt_db_attribute_read_result(attr, id, 0, value, n);
}

//void GattClient_onTimeout(int UNUSED_PARAM(fd), void* argp)
//{
//	GattClient* clnt = reinterpret_cast<GattClient *>(argp);
//	clnt->onTimeout();
//}


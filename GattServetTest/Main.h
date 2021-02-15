#ifndef MAIN_H
#define MAIN_H

// °Ñ¦Òbleconfd

#include <iostream>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <vector>

// these are pulled directly from the BlueZ source tree
extern "C"
{
#include <lib/uuid.h>
#include <src/shared/att.h>
#include <src/shared/gatt-db.h>
#include <src/shared/gatt-server.h>
}

extern "C"
{
#include <src/shared/mainloop.h>
}


#include "CommandToString.h"
#include "memory_stream.h"
#include "beacon.h"


char ServerName[64]{ "TheUnknownServer" };
char const      kRecordDelimiter{ 30 };
uint16_t const kUuidDeviceInfoService{ 0x180a };
static const uint16_t kUuidGap{ 0x1800 };
static const uint16_t kUuidGatt{ 0x1801 };

uint16_t const kUuidSystemId{ 0x2a23 };
uint16_t const kUuidModelNumber{ 0x2a24 };
uint16_t const kUuidSerialNumber{ 0x2a25 };
uint16_t const kUuidFirmwareRevision{ 0x2a26 };
uint16_t const kUuidHardwareRevision{ 0x2a27 };
uint16_t const kUuidSoftwareRevision{ 0x2a28 };
uint16_t const kUuidManufacturerName{ 0x2a29 };

std::string const kUuidRpcService{ "503553ca-eb90-11e8-ac5b-bb7e434023e8" };
std::string const kUuidRpcInbox{ "510c87c8-eb90-11e8-b3dc-17292c2ecc2d" };
std::string const kUuidRpcEPoll{ "5140f882-eb90-11e8-a835-13d2bd922d3f" };

std::string const kUuidTomofun{ "77f9a069-2ff9-4ff9-93b7-4db9689f4b53" };
std::string const Command_Characteristic_UUID =		{ "026048ab-fafc-48e5-9db0-a9bc46368055" };
std::string const Data_Characteristic_UUID =		{ "4cc2ee25-d059-4575-a4d8-d13565d7928c" }; // Write
std::string const Notify_Characteristic_UUID =		{ "30c95538-58f8-4db4-a62e-08af4bae4cb0" }; // Read Notify

bool m_service_change_enabled = false;
memory_stream       m_outgoing_queue(kRecordDelimiter);
std::vector<char>   m_incoming_buff;



// /defs.h
#define UNUSED_PARAM(X) UNUSED_ ## X __attribute__((__unused__))

using namespace std;

void GattClient_onGapRead(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp);

void GattClient_onGapWrite(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t const* data, size_t len, uint8_t opcode, bt_att* att, void* argp);

void GattClient_onServiceChanged(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp);

void GattClient_onServiceChangedRead(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp);

void GattClient_onServiceChangedWrite(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t const* value, size_t len, uint8_t opcode, bt_att* att, void* argp);

void GattClient_onGapExtendedPropertiesRead(gatt_db_attribute *attr, uint32_t id,
	uint16_t offset, uint8_t opcode, bt_att* att, void* argp);

void GattClient_onEPollRead(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp);

void GattClient_onDataChannelIn(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t const* data, size_t len, uint8_t opcode, bt_att* att, void* argp);

void GattClient_onDataChannelOut(gatt_db_attribute* attr, uint32_t id, uint16_t offset,
	uint8_t opcode, bt_att* att, void* argp);



void onClientDisconnected(int err, void* argp)
{
	// TODO: we should stash the remote client as a member of the
	// GattClient so we can print out mac addres of client that
	// just disconnected
	cout << "disconnect";
	mainloop_quit();
}

void onTimeout(int UNUSED_PARAM(fd), void* argp)
{
	cout << "timeout";
}



void DIS_writeCallback(gatt_db_attribute* UNUSED_PARAM(attr), int err, void* UNUSED_PARAM(argp))
{
	if (err)
	{
		printf("error writing to DIS service in GATT db. %d\n", err);
	}
}

/* buildGapService */
void buildGapService(gatt_db* m_db)
{
	bt_uuid_t uuid;
	bt_uuid16_create(&uuid, kUuidGap);

	gatt_db_attribute* service = gatt_db_add_service(m_db, &uuid, true, 6);

	// device name
	bt_uuid16_create(&uuid, GATT_CHARAC_DEVICE_NAME);
	gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
		BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_EXT_PROP,
		&GattClient_onGapRead, &GattClient_onGapWrite, nullptr);
		//nullptr, nullptr, nullptr);
		//&GattClient_onGapRead, &GattClient_onGapWrite, this);

	bt_uuid16_create(&uuid, GATT_CHARAC_EXT_PROPER_UUID);
	gatt_db_service_add_descriptor(service, &uuid, BT_ATT_PERM_READ,
		&GattClient_onGapExtendedPropertiesRead, nullptr, nullptr);
		//nullptr, nullptr, nullptr);
		//&GattClient_onGapExtendedPropertiesRead, nullptr, this);

	// appearance
	bt_uuid16_create(&uuid, GATT_CHARAC_APPEARANCE);
	gatt_db_attribute* attr = gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
		BT_GATT_CHRC_PROP_READ, nullptr, nullptr, nullptr);
		//BT_GATT_CHRC_PROP_READ, nullptr, nullptr, this);

	uint16_t appearance{ 0 };
	bt_put_le16(128, &appearance);
	gatt_db_attribute_write(attr, 0, reinterpret_cast<uint8_t const *>(&appearance),
		sizeof(appearance), BT_ATT_OP_WRITE_REQ, nullptr, &DIS_writeCallback, nullptr);
		//sizeof(appearance), BT_ATT_OP_WRITE_REQ, nullptr, nullptr, nullptr);
		//sizeof(appearance), BT_ATT_OP_WRITE_REQ, nullptr, &DIS_writeCallback, nullptr);

	gatt_db_service_set_active(service, true);
}





/* buildGattService */
void buildGattService(gatt_db* m_db)
{
	bt_uuid_t uuid;
	bt_uuid16_create(&uuid, kUuidGatt);

	gatt_db_attribute* service = gatt_db_add_service(m_db, &uuid, true, 4);

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVICE_CHANGED);
	gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
		BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_INDICATE,
		GattClient_onServiceChanged, nullptr, nullptr);
		//nullptr, nullptr, nullptr);
		//GattClient_onServiceChanged, nullptr, this);

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	gatt_db_service_add_descriptor(service, &uuid, BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
		GattClient_onServiceChangedRead, GattClient_onServiceChangedWrite, nullptr);
		//nullptr, nullptr, nullptr);
		//GattClient_onServiceChangedRead, GattClient_onServiceChangedWrite, this);

	gatt_db_service_set_active(service, true);
}


/* buildDeviceInfoService */

void addDeviceInfoCharacteristic(
	gatt_db_attribute* service,
	uint16_t           id,
	std::string const& value)
{
	bt_uuid_t uuid;
	bt_uuid16_create(&uuid, id);

	gatt_db_attribute* attr = gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
		BT_GATT_CHRC_PROP_READ, nullptr, nullptr, nullptr);
		//BT_GATT_CHRC_PROP_READ, nullptr, nullptr, this);

	if (!attr)
	{
		cout << "failed to create DIS characteristic" << id << endl;
		return;
	}

	uint8_t const* p = reinterpret_cast<uint8_t const *>(value.c_str());


	cout << "setting DIS attr:" << id << "to" << value << endl;

	// I'm not sure whether i like this or just having callbacks setup for reads
	gatt_db_attribute_write(attr, 0, p, value.length(), BT_ATT_OP_WRITE_REQ, nullptr,
		nullptr, nullptr);
		//&DIS_writeCallback, nullptr);

}


void buildDeviceInfoService(gatt_db* m_db)
{
	bt_uuid_t uuid;
	bt_uuid16_create(&uuid, kUuidDeviceInfoService);

	// TODO: I don't know what the end value is for. The last call here to add
	// the manufacturer name was failing, but when I upp'ed it from 14 to 30
	// the error went away. Not sure what's going on?
	gatt_db_attribute* service = gatt_db_add_service(m_db, &uuid, true, 30);




	addDeviceInfoCharacteristic(service, kUuidSystemId, exec("cat /etc/machine-id"));
	addDeviceInfoCharacteristic(service, kUuidModelNumber, exec("cat /proc/device-tree/model"));
	//addDeviceInfoCharacteristic(service, kUuidSerialNumber, deviceInfoProvider.GetSerialNumber());
	addDeviceInfoCharacteristic(service, kUuidFirmwareRevision, exec("uanme -a"));
	//addDeviceInfoCharacteristic(service, kUuidHardwareRevision, deviceInfoProvider.GetHardwareRevision());
	//addDeviceInfoCharacteristic(service, kUuidSoftwareRevision, deviceInfoProvider.GetSoftwareRevision());
	//addDeviceInfoCharacteristic(service, kUuidManufacturerName, deviceInfoProvider.GetManufacturerName());

	gatt_db_service_set_active(service, true);
}

void buildRpcService(gatt_db* m_db)
{
	bt_uuid_t uuid;

	bt_string_to_uuid(&uuid, kUuidRpcService.c_str());
	gatt_db_attribute* service = gatt_db_add_service(m_db, &uuid, true, 25);
	if (!service)
	{
		printf("failed to add rpc service to gatt db");
	}

	// data channel
	bt_string_to_uuid(&uuid, kUuidRpcInbox.c_str());
	gatt_db_attribute* m_data_channel = gatt_db_service_add_characteristic(
		service,
		&uuid,
		BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
		BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_WRITE,
		&GattClient_onDataChannelOut,
		&GattClient_onDataChannelIn,
		nullptr);

	if (!m_data_channel)
	{
		printf("failed to create inbox characteristic");
	}

	// blepoll
	//bt_string_to_uuid(&uuid, kUuidRpcEPoll.c_str());
	bt_string_to_uuid(&uuid, Notify_Characteristic_UUID.c_str());
	gatt_db_attribute* m_blepoll = gatt_db_service_add_characteristic(
		service,
		&uuid,
		//BT_ATT_PERM_NONE,
		//BT_GATT_CHRC_PROP_NOTIFY,
		//NULL, NULL, NULL);
		BT_ATT_PERM_READ,
		BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_NOTIFY,
		&GattClient_onEPollRead,
		nullptr,
		nullptr);

	/*
	uint16_t m_notify_handle = gatt_db_attribute_get_handle(m_blepoll);

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	gatt_db_service_add_descriptor(
		service,
		&uuid,
		BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
		&GattClient_onEPollRead,
		nullptr,
		nullptr);

	if (!m_blepoll)
	{
		printf("failed to create ble poll indicator characteristic");
	}
	*/

	gatt_db_service_set_active(service, true);
}


void tomo_command_write_cb(struct gatt_db_attribute *attrib,
	unsigned int id, uint16_t offset,
	const uint8_t *value, size_t len,
	uint8_t opcode, struct bt_att *att,
	void *user_data) {
	printf("tomo_command_write_cb\n");
}

void tomo_data_read_cb(struct gatt_db_attribute *attrib,
	unsigned int id, uint16_t offset,
	uint8_t opcode, struct bt_att *att,
	void *user_data){
	printf("tomo_data_read_cb\n");
}

void tomo_data_write_cb(struct gatt_db_attribute *attrib,
	unsigned int id, uint16_t offset,
	const uint8_t *value, size_t len,
	uint8_t opcode, struct bt_att *att,
	void *user_data){
	printf("tomo_data_write_cb\n");
}


void buildTomofunService(gatt_db* m_db) {

	bt_uuid_t uuid;
	gatt_db_attribute *service;


	service = gatt_db_add_service(m_db, &uuid, true, 25);
	//server->hr_handle = gatt_db_attribute_get_handle(service);
	
	if (!service)
	{
		printf("failed to add tomofun service to gatt db");
	}

	printf("Command_Characteristic_UUID\n");
	/*Command Characteristic */
	bt_string_to_uuid(&uuid, Command_Characteristic_UUID.c_str());
	
	gatt_db_service_add_characteristic(service, &uuid,
		BT_ATT_PERM_WRITE,
		BT_GATT_CHRC_PROP_WRITE,
		NULL,
		&tomo_command_write_cb, NULL);// server);
	
	printf("Data_Characteristic_UUID\n");
	/*Data Characteristic */
	bt_string_to_uuid(&uuid, Data_Characteristic_UUID.c_str());
	
	gatt_db_service_add_characteristic(service, &uuid,
		BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
		BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_WRITE,
		&tomo_data_read_cb, &tomo_data_write_cb, NULL);// server);
	
	printf("Notify_Characteristic_UUID\n");
	/*Notify Characteristic */
	bt_string_to_uuid(&uuid, Notify_Characteristic_UUID.c_str());
	
	gatt_db_attribute *tomo_notify = gatt_db_service_add_characteristic(service, &uuid,
		//BT_ATT_PERM_READ,
		//BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_NOTIFY,
		//gatt_service_changed_cb, NULL, server);
		BT_ATT_PERM_NONE,
		BT_GATT_CHRC_PROP_NOTIFY,
		NULL, NULL, NULL);
	//
	//printf("tomo_notify_handle\n");
	//uint16_t tomo_notify_handle; 
	//tomo_notify_handle = gatt_db_attribute_get_handle(tomo_notify);
	//
	//printf("GATT_CLIENT_CHARAC_CFG_UUID\n");
	///*Needed by Notify Characteristic.*/
	//bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	//gatt_db_service_add_descriptor(service, &uuid,
	//	BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
	//	NULL,
	//	NULL, NULL); //server);



	gatt_db_service_set_active(service, true);




}


#endif
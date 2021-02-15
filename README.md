# GattServerTest

https://github.com/gladish/bleconfd

g++ Main.cpp -o test -I/home/pi/bleconfd/build/deps/src/bluez -lbluetooth /home/pi/bleconfd/build/deps/src/bluez/src/.libs/libshared-mainloop.a /home/pi/bleconfd/build/deps/src/bluez/lib/.libs/uuid.o

sudo hciconfig hci0 leadv 0

在att.c最後加一行
int bt_att_write_queue_size(struct bt_att *att){
	
	return queue_length(att->write_queue);
}

att.h加一行
int bt_att_write_queue_size(struct bt_att *att);

在gatt-server.c加一行
int bt_gatt_server_get_write_queue_length(struct bt_gatt_server *server){
	return bt_att_write_queue_size(server->att);
}
	
在gatt-server.h加一行
int bt_gatt_server_get_write_queue_length(struct bt_gatt_server *server);

然後重新compile bluez，再拿他的libshared-mainloop.a和uuid.o出來用
g++ beacon.cc -c -lbluetooth
g++ util.cc -c

g++ Main.cpp -o test -pthread -I/home/pi/bleconfd/build/deps/src/bluez -lbluetooth /home/pi/bleconfd/build/deps/src/bluez/src/.libs/libshared-mainloop.a /home/pi/bleconfd/build/deps/src/bluez/lib/.libs/uuid.o beacon.o util.o

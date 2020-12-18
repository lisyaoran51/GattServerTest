#ifndef MAIN_H
#define MAIN_H




void
onClientDisconnected(int err)
{
	// TODO: we should stash the remote client as a member of the
	// GattClient so we can print out mac addres of client that
	// just disconnected
	XLOG_INFO("disconnect:%d", err);
	mainloop_quit();
}



#endif
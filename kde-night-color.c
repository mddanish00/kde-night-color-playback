#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <QtDBus>

#include <mpv/client.h>

void inhibit_nc(){
	QDBusInterface iface(
		"org.kde.KWin",
		"/org/kde/KWin/NightLight",
		"org.kde.KWin.NightLight",
		QDBusConnection::sessionBus()
	);

	iface.call("inhibit");
}

extern "C"{
	int mpv_open_cplugin(mpv_handle *handle){
		inhibit_nc();

		while (true)
		{
			mpv_event *event = mpv_wait_event(handle, 1);
			if (event->event_id == MPV_EVENT_SHUTDOWN)
				break;
		}
		return 0;
	}
}

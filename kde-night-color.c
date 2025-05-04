#include <stddef.h>

#include <stdlib.h>

#include <QtDBus>

#include <mpv/client.h>

// Added: store inhibition token
uint night_cookie = 0;

void inhibit_nc(bool inhibit)
{
	QDBusInterface iface(
		"org.kde.KWin",
		"/org/kde/KWin/NightLight",
		"org.kde.KWin.NightLight",
		QDBusConnection::sessionBus());

	if (inhibit)
	{
		QDBusReply<uint> reply = iface.call("inhibit");
		if (reply.isValid())
			night_cookie = reply.value();
	}
	else if (night_cookie != 0)
	{
		iface.call("uninhibit", night_cookie);
		night_cookie = 0;
	}
}

extern "C"
{
	int mpv_open_cplugin(mpv_handle *handle)
	{
		bool night_light_inhibited = false;

		auto update_inhibition = [&]()
		{
			int64_t paused = 1, idle = 1;
			mpv_get_property(handle, "pause", MPV_FORMAT_FLAG, &paused);
			mpv_get_property(handle, "core-idle", MPV_FORMAT_FLAG, &idle);
			int64_t seeking = 0;
			mpv_get_property(handle, "seeking", MPV_FORMAT_FLAG, &seeking);

			bool should_inhibit = (seeking || !idle) && !paused;
			if (should_inhibit != night_light_inhibited)
			{
				inhibit_nc(should_inhibit);
				night_light_inhibited = should_inhibit;
			}
		};

		// Observe pause and idle properties
		mpv_observe_property(handle, 0, "pause", MPV_FORMAT_FLAG);
		mpv_observe_property(handle, 0, "core-idle", MPV_FORMAT_FLAG);
		// Also observe seeking to prevent flicker during jumps
		mpv_observe_property(handle, 0, "seeking", MPV_FORMAT_FLAG);

		while (true)
		{
			mpv_event *event = mpv_wait_event(handle, -1); // Blocking wait: more efficient than polling
			// We wait indefinitely for property changes (pause, core-idle), avoiding CPU waste
			if (!event || event->event_id == MPV_EVENT_NONE)
				continue;
			if (event->event_id == MPV_EVENT_SHUTDOWN)
				break;
			if (event->event_id == MPV_EVENT_PROPERTY_CHANGE)
			{
				mpv_event_property *prop = (mpv_event_property *)event->data;
				if (strcmp(prop->name, "pause") == 0 || strcmp(prop->name, "core-idle") == 0 || strcmp(prop->name, "seeking") == 0)
				{
					update_inhibition();
				}
			}
		}

		if (night_light_inhibited)
			inhibit_nc(false);

		return 0;
	}
}

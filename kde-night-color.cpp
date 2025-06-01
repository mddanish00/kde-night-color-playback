#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dbus/dbus.h>
#include <mpv/client.h>

// Constants
const char *KWIN_SERVICE = "org.kde.KWin";
const char *KWIN_OBJECT_PATH = "/org/kde/KWin/NightLight";
const char *KWIN_INTERFACE = "org.kde.KWin.NightLight";

// Global variables
uint32_t night_cookie = 0;
static DBusConnection *g_dbus_conn = nullptr;
const int DBUS_TIMEOUT_MS = 5000;

void inhibit_nc(bool inhibit)
{
	// Connect to session bus once
	if (!g_dbus_conn)
	{
		DBusError err;
		dbus_error_init(&err);
		g_dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
		if (dbus_error_is_set(&err))
		{
			fprintf(stderr, "[kde-night-color-playback] DBus Connection Error: %s\n", err.message);
			dbus_error_free(&err);
			return;
		}
	}

	if (inhibit)
	{
		DBusMessage *msg, *reply;
		DBusError err;
		dbus_error_init(&err);

		msg = dbus_message_new_method_call(
			KWIN_SERVICE,
			KWIN_OBJECT_PATH,
			KWIN_INTERFACE,
			"inhibit");

		if (!msg)
		{
			fprintf(stderr, "[kde-night-color-playback] DBus Message Error: Failed to create inhibit message.\n");
			return;
		}

		reply = dbus_connection_send_with_reply_and_block(g_dbus_conn, msg, DBUS_TIMEOUT_MS, &err);
		dbus_message_unref(msg);

		if (dbus_error_is_set(&err))
		{
			if (dbus_error_has_name(&err, DBUS_ERROR_TIMEOUT) || dbus_error_has_name(&err, DBUS_ERROR_NO_REPLY))
			{
				fprintf(stderr, "[kde-night-color-playback] DBus Call Error: Timeout waiting for reply from KWin (%s).\n", err.message);
			}
			else
			{
				fprintf(stderr, "[kde-night-color-playback] DBus Call Error: %s\n", err.message);
			}
			dbus_error_free(&err);
			return;
		}

		uint32_t cookie = 0;
		if (dbus_message_get_args(reply, &err, DBUS_TYPE_UINT32, &cookie, DBUS_TYPE_INVALID))
		{
			night_cookie = cookie;
		}
		else
		{
			fprintf(stderr, "[kde-night-color-playback] DBus Reply Error: %s\n", err.message);
			dbus_error_free(&err);
			night_cookie = 0; // Explicitly indicate failure to obtain a new cookie
		}
		dbus_message_unref(reply);
	}
	else if (night_cookie != 0)
	{
		DBusMessage *msg;
		msg = dbus_message_new_method_call(
			KWIN_SERVICE,
			KWIN_OBJECT_PATH,
			KWIN_INTERFACE,
			"uninhibit");

		if (!msg)
		{
			fprintf(stderr, "[kde-night-color-playback] DBus Message Error: Failed to create uninhibit message.\n");
			return;
		}

		if (dbus_message_append_args(msg, DBUS_TYPE_UINT32, &night_cookie, DBUS_TYPE_INVALID))
		{
			if (!dbus_connection_send(g_dbus_conn, msg, nullptr))
			{
				fprintf(stderr, "[kde-night-color-playback] DBus Send Error: Failed to send uninhibit message.\n");
			}
			else
			{
				night_cookie = 0;
			}
		}
		else
		{
			fprintf(stderr, "[kde-night-color-playback] DBus Append Args Error: Failed to append arguments for uninhibit. Message not sent.\n");
		}
		dbus_message_unref(msg);
	}
}

void cleanup_dbus_resources()
{
	if (g_dbus_conn)
	{
		dbus_connection_unref(g_dbus_conn);
		g_dbus_conn = nullptr;
	}
}

// Helper function to get an mpv property (MPV_FORMAT_FLAG)
// Returns true on success, false on error (and prints an error message).
static bool get_mpv_property_flag(mpv_handle *handle, const char *property_name, int *out_value) {
    int err = mpv_get_property(handle, property_name, MPV_FORMAT_FLAG, out_value);
    if (err != MPV_ERROR_SUCCESS) {
        fprintf(stderr, "[kde-night-color-playback] Error getting '%s' property: %s. Skipping update.\n", property_name, mpv_error_string(err));
        return false;
    }
    return true;
}

extern "C"
{
	int mpv_open_cplugin(mpv_handle *handle)
	{
		bool night_light_inhibited = false;

		auto update_inhibition = [&]()
		{
			int paused, idle, seeking, paused_for_cache;

			if (!get_mpv_property_flag(handle, "pause", &paused)) return;
			if (!get_mpv_property_flag(handle, "core-idle", &idle)) return;
			if (!get_mpv_property_flag(handle, "seeking", &seeking)) return;
			if (!get_mpv_property_flag(handle, "paused-for-cache", &paused_for_cache)) return;

			bool should_inhibit = (!paused && !idle) || paused_for_cache || seeking;
			if (should_inhibit != night_light_inhibited)
			{
				inhibit_nc(should_inhibit);
				// Update night_light_inhibited based on the actual outcome of inhibit_nc,
				// which is reflected by whether night_cookie is non-zero.
				// If inhibit_nc(true) succeeded, night_cookie is non-zero.
				// If inhibit_nc(true) failed, night_cookie is zero.
				// If inhibit_nc(false) succeeded, night_cookie is zero.
				// If inhibit_nc(false) failed (e.g. send error), night_cookie remains non-zero.
				// This correctly updates our local state to match what we believe the inhibition state to be.
				night_light_inhibited = (night_cookie != 0);
			}

			// Add logging of current state
			printf("[kde-night-color-playback] CurrentState= pause: %d, core-idle: %d, seeking: %d, paused-for-cache: %d, inhibited: %d\n",
				   paused, idle, seeking, paused_for_cache, night_light_inhibited);
			fflush(stdout);
		};

		// Observe properties
		mpv_observe_property(handle, 0, "pause", MPV_FORMAT_FLAG);
		mpv_observe_property(handle, 0, "core-idle", MPV_FORMAT_FLAG);
		mpv_observe_property(handle, 0, "seeking", MPV_FORMAT_FLAG);
		mpv_observe_property(handle, 0, "paused-for-cache", MPV_FORMAT_FLAG);

		// Call once to set initial state based on current mpv properties
		update_inhibition();

		while (true)
		{
			mpv_event *event = mpv_wait_event(handle, -1); // Blocking wait
			if (!event || event->event_id == MPV_EVENT_NONE)
				continue;
			if (event->event_id == MPV_EVENT_SHUTDOWN)
				break;
			if (event->event_id == MPV_EVENT_PROPERTY_CHANGE)
			{
				mpv_event_property *prop = (mpv_event_property *)event->data;
				if (strcmp(prop->name, "pause") == 0 || strcmp(prop->name, "core-idle") == 0 ||
					strcmp(prop->name, "seeking") == 0 || strcmp(prop->name, "paused-for-cache") == 0)
				{
					update_inhibition();
				}
			}
		}

		if (night_light_inhibited)
			inhibit_nc(false);

		cleanup_dbus_resources();

		return 0;
	}
}

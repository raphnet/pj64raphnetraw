/* mupen64plus-input-raphnetraw
 *
 * Copyright (C) 2016 Raphael Assenat
 *
 * An input plugin that lets the game under emulation communicate with
 * the controllers directly using the direct controller communication
 * feature of raphnet V3 adapters[1].
 *
 * [1] http://www.raphnet.net/electronique/gcn64_usb_adapter_gen3/index_en.php
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <stdio.h>
#include "plugin_back.h"
#include "gcn64.h"
#include "gcn64lib.h"
#include "version.h"
#define ENABLE_TIMING

#ifdef ENABLE_TIMING
#include <sys/time.h>
#endif

//#define TIME_RAW_IO
#define TIME_COMMAND_TO_READ

static void nodebug(int l, const char *m, ...) { }

static pb_debugFunc DebugMessage = nodebug;
static int g_adapter_n_channels = 2;
static gcn64_hdl_t gcn64_handle = NULL;

int pb_init(pb_debugFunc debugFn)
{
	DebugMessage = debugFn;
	gcn64_init(1);
	return 0;
}

int pb_shutdown(void)
{
	if (gcn64_handle) {
		/* RomClosed() should have done this, but just
		   in case it is not always called, do this again here. */
		gcn64lib_suspendPolling(gcn64_handle, 0);

		gcn64_closeDevice(gcn64_handle);
	}

	gcn64_shutdown();
	return 0;
}

int pb_scanControllers(void)
{
	struct gcn64_list_ctx * lctx;
	struct gcn64_info inf;

	lctx = gcn64_allocListCtx();
	if (!lctx) {
		DebugMessage(PB_MSG_ERROR, "Could not allocate gcn64 list context\n");
		return 0;
	}

	/* This uses the first device we are able to open. TODO : Configurable per serial */
	while (gcn64_listDevices(&inf, lctx)) {
		gcn64_handle = gcn64_openDevice(&inf);
		if (!gcn64_handle) {
			DebugMessage(PB_MSG_ERROR, "Could not open gcn64 device\n");
		}

		DebugMessage(PB_MSG_INFO, "Using USB device 0x%04x:0x%04x serial '%ls' name '%ls'",
						inf.usb_vid, inf.usb_pid, inf.str_serial, inf.str_prodname);

		break;
	}
	gcn64_freeListCtx(lctx);

	if (gcn64_handle) {

		g_adapter_n_channels = inf.caps.n_raw_channels;
		DebugMessage(PB_MSG_INFO, "Adapter supports %d raw channels", g_adapter_n_channels);

		return g_adapter_n_channels;
	}


	return 0;
}

int pb_romOpen(void)
{
	if (gcn64_handle) {
		gcn64lib_suspendPolling(gcn64_handle, 1);
	}
	return 0;
}

int pb_romClosed(void)
{
	if (gcn64_handle) {
		gcn64lib_suspendPolling(gcn64_handle, 0);
	}
	return 0;
}

static void timing(int start, const char *label)
{
#ifdef ENABLE_TIMING
	static struct timeval tv_start;
	static int started = 0;
	struct timeval tv_now;

	if (start) {
		gettimeofday(&tv_start, NULL);
		started = 1;
		return;
	}

	if (started) {
		gettimeofday(&tv_now, NULL);
		printf("%s: %ld us\n", label, (tv_now.tv_sec - tv_start.tv_sec) * 1000000 + (tv_now.tv_usec - tv_start.tv_usec) );
		started = 0;
	}
#endif
}

#ifdef _DEBUG
static void debug_raw_commands_in(unsigned char *command, int channel_id)
{
	int tx_len = command[0];
	int rx_len = command[1] & 0x3F;
	int i;

	printf("chn %d: tx[%d] = {", channel_id, tx_len);
	for (i=0; i<tx_len; i++) {
		printf("0x%02x ", command[2+i]);
	}
	printf("}, expecting %d byte%c\n", rx_len, rx_len > 1 ? 's':' ');

}

static void debug_raw_commands_out(unsigned char *command, int channel_id)
{
	int result = command[1];
	int i;

	printf("chn %d: result: 0x%02x : ", channel_id, result);
	if (0 == (command[1] & 0xC0)) {
		for (i=0; i<result; i++) {
			printf("0x%02x ", command[2+i]);
		}
	} else {
		printf("error\n");
	}
	printf("\n");
}

#endif


int pb_controllerCommand(int Control, unsigned char *Command)
{
	// I thought of sending requests to the adapter from here, reading
	// the results later in readController.
	//
	// But unlike what I expected, controllerCommand is NOT always called
	// before readController... When it is, readController() is only called
	// about 100us later. Not worth pursing for now.
#ifdef TIME_COMMAND_TO_READ
	timing(1, NULL);
#endif
	return 0;
}

int pb_readController(int Control, unsigned char *Command)
{
#ifdef TIME_COMMAND_TO_READ
	timing(0, "Command to Read");
#endif
	if (Control != -1 && Command) {

		// Command structure
		// based on LaC's n64 hardware dox... v0.8 (unofficial release)
		//
		// Byte 0: nBytes to transmit
		// Byte 1: nBytes to receive
		// Byte 2: TX data start (first byte is the command)
		// Byte 2+[tx_len]: RX data start
		//
		// Request info (status):
		//
		// tx[1] = {0x00 }, expecting 3 bytes
		//
		// Get button status:
		//
		// tx[1] = {0x01 }, expecting 4 bytes
		//
		// Rumble commands: (Super smash bros)
		//
		// tx[35] = {0x03 0x80 0x01 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe
		// 			0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe
		// 			0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe 0xfe }, expecting 1 byte
		//
		// tx[3] = {0x02 0x80 0x01 }, expecting 33 bytes
		//
		// Memory card commands: (Mario Kart 64)
		//
		// tx[3] = {0x02 0x00 0x35 }, expecting 33 bytes
		// tx[35] = {0x03 0x02 0x19 0x00 0x71 0xbf 0x95 0xf5 0xfb 0xd7 0xc1 0x00 0x00
		// 			0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00
		// 			0x03 0x00 0x03 0x00 0x03 0x00 0x03 0x00 0x03 }, expecting 1 byte
		if (gcn64_handle) {
			int tx_len = Command[0];
			int rx_len = Command[1];
			int res;

#ifdef _DEBUG
			debug_raw_commands_in(Command, Control);
#endif

			if (!rx_len) {
				return 0;
			}

			// When a CIC challenge took place in update_pif_write(), the pif ram
			// contains a bunch 0xFF followed by 0x00 at offsets 46 and 47. Offset
			// 48 onwards contains the challenge answer.
			//
			// Then when update_pif_read() is called, the 0xFF bytes are be skipped
			// up to the two 0x00 bytes that increase the channel to 2. Then the
			// challenge answer is (incorrectly) processed as if it were commands
			// for the third controller.
			//
			// This cause issues with the raphnetraw plugin since it modifies pif ram
			// to store the result or command error flags. This corrupts the reponse
			// and leads to challenge failure.
			//
			// As I know of no controller commands above 0x03, the filter below guards
			// against this condition...
			//
			if (Control == 2 && Command[2] > 0x03) {
				DebugMessage(PB_MSG_WARNING, "Invalid controller command\n");
				return 0;
			}
#ifdef TIME_RAW_IO
			timing(1, NULL);
#endif
			res = gcn64lib_rawSiCommand(gcn64_handle,
									Control,		// Channel
									Command + 2,	// TX data
									tx_len,		// TX data len
									Command + 2 + tx_len,	// RX data
									rx_len		// TX data len
								);
#ifdef TIME_RAW_IO
			timing(0, "rawCommand");
#endif

			if (res <= 0) {
				// 0x00 - no error, operation successful.
				// 0x80 - error, device not present for specified command.
				// 0x40 - error, unable to send/recieve the number bytes for command type.
				//
				// Diagrams 1.2-1.4 shows that lower bits are preserved (i.e. The length
				// byte is not completely replaced).
				Command[1] &= ~0xC0;
				Command[1] |= 0x80;
			} else {
				// Success. Make sure error bits are clear, if any.
				Command[1] &= ~0xC0;
			}

#ifdef _DEBUG
			debug_raw_commands_out(Command, Control);
#endif
		}
	}

	return 0;
}


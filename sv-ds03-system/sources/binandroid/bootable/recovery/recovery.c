/****************************************************************************

*  Copyright (C) 2015 Cambridge Silicon Radio Limited

*

*  This software is based on third party software that has been modified by CSR.

*  The underlying software was licensed by the author under separate terms,

*  as provided in the original license statement for your information only.

*

*  Notwithstanding, this software, when distributed to you by CSR, is licensed

*  to you by CSR under the terms of the proprietary CSR license under which the

*  software was delivered to you, and not under the original author’s terms.

*

****************************************************************************/

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "install.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"

static const struct option OPTIONS[] = {
	{ "send_intent", required_argument, NULL, 's' },
	{ "update_package", required_argument, NULL, 'u' },
	{ "wipe_data", no_argument, NULL, 'w' },
	{ "wipe_cache", no_argument, NULL, 'c' },
	{ "show_text", no_argument, NULL, 't' },
	{ NULL, 0, NULL, 0 },
};

static const char *COMMAND_FILE = "/sdcard/cache/recovery/command";
static const char *INTENT_FILE = "/sdcard/cache/recovery/intent";
static const char *LOG_FILE = "/sdcard/cache/recovery/log";
static const char *LAST_LOG_FILE = "/sdcard/cache/recovery/last_log";
static const char *LAST_INSTALL_FILE = "/sdcard/cache/recovery/last_install";
static const char *CACHE_ROOT = "/sdcard/cache";
static const char *SDCARD_ROOT = "/sdcard";
static const char *TEMPORARY_LOG_FILE = "/tmp/recovery.log";
static const char *TEMPORARY_INSTALL_FILE = "/tmp/last_install";
static const char *SIDELOAD_TEMP_DIR = "/tmp/sideload";

static const char *UPDATE_PACKAGE_PATH = "/sdcard/update.zip";
static const char *UPDATE_PACKAGE_PATH_EXT = "/sdcard/card/update.zip";
static const char *UPDATEZIP_PATH = "/user/update.zip";
static const char *UPDATESVP_PATH = "/user/svp.bin";
static const char *UPDATEAPP_PATH = "/user/app-pkg.bin";
static const char *UPDATE_PACKAGE_SVP_PATH_EXT = "/sdcard/svp.bin";
static const char *UPDATE_PACKAGE_APP_PATH_EXT = "/sdcard/app-pkg.bin";

extern UIParameters ui_parameters;    // from ui.c

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#define NO_UPDATE               0
#define MOUDLE_UPDATE           1
#define FACTORY_UPDATE          2

#define SIZE_1K                 1024
#define SIZE_1M                 (1024*1024)
#define SECTOR_SIZE             512

#define OEMDATA_OFFSET          (5*SIZE_1M)

#define RAW_ZONE                "/dev/mmcblk0"

#define SVP_PKG_NAME            "svp.bin"
#define APP_PKG_NAME            "app-pkg.bin"
#define OS_PKG_NAME             "update.zip"

#define UPDATE_MEDIA            "/media/"
#define EMMC_UPDATE_MEDIA       "/media/mmcblk0p8"
#define USB_MEDIA               "usbstorage"
#define SD_MEDIA                "sdcard"

#define  UPDATE_MAGIC           'ota1'

typedef union update_union {
	struct {
		unsigned char os : 1;
		unsigned char svp : 1;
		unsigned char app : 1;
		unsigned char logo : 1;
		unsigned char radio : 1;
		unsigned char dsp : 1;
		unsigned char gps : 1;
		unsigned int reserved0 : 9;
		unsigned char mlo : 1;
		unsigned char uboot : 1;
		unsigned char dtb : 1;
		unsigned char uimage : 1;
		unsigned char targetfs : 1;
		unsigned char dtb_1g5 : 1;
		unsigned int reserved1 : 10;
	};
	unsigned int flag;
} PACKED update_t;

typedef struct oemdata_struct {
	int magic;
	update_t update;
} PACKED oemdata_t;


/*
 * The recovery tool communicates with the main system through /cache files.
 *   /cache/recovery/command - INPUT - command line for tool, one arg per line
 *   /cache/recovery/log - OUTPUT - combined log file from recovery run(s)
 *   /cache/recovery/intent - OUTPUT - intent that was passed in
 *
 * The arguments which may be supplied in the recovery.command file:
 *   --send_intent=anystring - write the text out to recovery.intent
 *   --update_package=path - verify install an OTA package file
 *   --wipe_data - erase user data (and cache), then reboot
 *   --wipe_cache - wipe cache (but not user data), then reboot
 *   --set_encrypted_filesystem=on|off - enables / diasables encrypted fs
 *
 * After completing, we remove /cache/recovery/command and reboot.
 * Arguments may also be supplied in the bootloader control block (BCB).
 * These important scenarios must be safely restartable at any point:
 *
 * FACTORY RESET
 * 1. user selects "factory reset"
 * 2. main system writes "--wipe_data" to /cache/recovery/command
 * 3. main system reboots into recovery
 * 4. get_args() writes BCB with "boot-recovery" and "--wipe_data"
 *    -- after this, rebooting will restart the erase --
 * 5. erase_volume() reformats /data
 * 6. erase_volume() reformats /cache
 * 7. finish_recovery() erases BCB
 *    -- after this, rebooting will restart the main system --
 * 8. main() calls reboot() to boot main system
 *
 * OTA INSTALL
 * 1. main system downloads OTA package to /cache/some-filename.zip
 * 2. main system writes "--update_package=/cache/some-filename.zip"
 * 3. main system reboots into recovery
 * 4. get_args() writes BCB with "boot-recovery" and "--update_package=..."
 *    -- after this, rebooting will attempt to reinstall the update --
 * 5. install_package() attempts to install the update
 *    NOTE: the package install must itself be restartable from any point
 * 6. finish_recovery() erases BCB
 *    -- after this, rebooting will (try to) restart the main system --
 * 7. ** if install failed **
 *    7a. prompt_and_wait() shows an error icon and waits for the user
 *    7b; the user reboots (pulling the battery, etc) into the main system
 * 8. main() calls maybe_install_firmware_update()
 *    ** if the update contained radio/hboot firmware **:
 *    8a. m_i_f_u() writes BCB with "boot-recovery" and "--wipe_cache"
 *        -- after this, rebooting will reformat cache & restart main system --
 *    8b. m_i_f_u() writes firmware image into raw cache partition
 *    8c. m_i_f_u() writes BCB with "update-radio/hboot" and "--wipe_cache"
 *        -- after this, rebooting will attempt to reinstall firmware --
 *    8d. bootloader tries to flash firmware
 *    8e. bootloader writes BCB with "boot-recovery" (keeping "--wipe_cache")
 *        -- after this, rebooting will reformat cache & restart main system --
 *    8f. erase_volume() reformats /cache
 *    8g. finish_recovery() erases BCB
 *        -- after this, rebooting will (try to) restart the main system --
 * 9. main() calls reboot() to boot main system
 */

static const int MAX_ARG_LENGTH = 4096;
static const int MAX_ARGS = 100;

// open a given path, mounting partitions as necessary
FILE*
fopen_path(const char *path, const char *mode)
{
	if (ensure_path_mounted(path) != 0) {
		LOGE("Can't mount %s\n", path);
		return NULL;
	}

	// When writing, try to create the containing directory, if necessary.
	// Use generous permissions, the system (init.rc) will reset them.
	if (strchr("wa", mode[0])) dirCreateHierarchy(path, 0777, NULL, 1);

	FILE *fp = fopen(path, mode);
	return fp;
}

// close a file, log an error if the error indicator is set
static void
check_and_fclose(FILE *fp, const char *name)
{
	fflush(fp);
	if (ferror(fp)) LOGE("Error in %s\n(%s)\n", name, strerror(errno));
	fclose(fp);
}

// command line args come from, in decreasing precedence:
//   - the actual command line
//   - the bootloader control block (one per line, after "recovery")
//   - the contents of COMMAND_FILE (one per line)
static void
get_args(int *argc, char ***argv)
{
	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
	get_bootloader_message(&boot);  // this may fail, leaving a zeroed structure

	if (boot.command[0] != 0 && boot.command[0] != 255) {
		LOGI("Boot command: %.*s\n", sizeof(boot.command), boot.command);
	}

	if (boot.status[0] != 0 && boot.status[0] != 255) {
		LOGI("Boot status: %.*s\n", sizeof(boot.status), boot.status);
	}

	// --- if arguments weren't supplied, look in the bootloader control block
	if (*argc <= 1) {
		boot.recovery[sizeof(boot.recovery) - 1] = '\0';  // Ensure termination
		const char *arg = strtok(boot.recovery, "\n");
		if (arg != NULL && !strcmp(arg, "recovery")) {
			*argv = (char **) malloc(sizeof(char *) * MAX_ARGS);
			(*argv)[0] = strdup(arg);
			for (*argc = 1; *argc < MAX_ARGS; ++*argc) {
				if ((arg = strtok(NULL, "\n")) == NULL) break;
				(*argv)[*argc] = strdup(arg);
			}
			LOGI("Got arguments from boot message\n");
		} else if (boot.recovery[0] != 0 && boot.recovery[0] != 255) {
			LOGE("Bad boot message\n\"%.20s\"\n", boot.recovery);
		}
	}

	// --- if that doesn't work, try the command file
	if (*argc <= 1) {
		FILE *fp = fopen_path(COMMAND_FILE, "r");
		if (fp != NULL) {
			char *argv0 = (*argv)[0];
			*argv = (char **) malloc(sizeof(char *) * MAX_ARGS);
			(*argv)[0] = argv0;  // use the same program name

			char buf[MAX_ARG_LENGTH];
			for (*argc = 1; *argc < MAX_ARGS; ++*argc) {
				if (!fgets(buf, sizeof(buf), fp)) break;
				(*argv)[*argc] = strdup(strtok(buf, "\r\n"));  // Strip newline.
			}

			check_and_fclose(fp, COMMAND_FILE);
			LOGI("Got arguments from %s\n", COMMAND_FILE);
		}
	}

	// --> write the arguments we have back into the bootloader control block
	// always boot into recovery after this (until finish_recovery() is called)
	strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
	strlcpy(boot.recovery, "recovery\n", sizeof(boot.recovery));
	int i;
	for (i = 1; i < *argc; ++i) {
		strlcat(boot.recovery, (*argv)[i], sizeof(boot.recovery));
		strlcat(boot.recovery, "\n", sizeof(boot.recovery));
	}
	set_bootloader_message(&boot);
}

static void
set_sdcard_update_bootloader_message()
{
	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
	strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
	strlcpy(boot.recovery, "recovery\n", sizeof(boot.recovery));
	set_bootloader_message(&boot);
}

// How much of the temp log we have copied to the copy in cache.
static long tmplog_offset = 0;

static void
copy_log_file(const char* source, const char* destination, int append)
{
	FILE *log = fopen_path(destination, append ? "a" : "w");
	if (log == NULL) {
		LOGE("Can't open %s\n", destination);
	} else {
		FILE *tmplog = fopen(source, "r");
		if (tmplog != NULL) {
			if (append) {
				fseek(tmplog, tmplog_offset, SEEK_SET);  // Since last write
			}
			char buf[4096];
			while (fgets(buf, sizeof(buf), tmplog)) fputs(buf, log);
			if (append) {
				tmplog_offset = ftell(tmplog);
			}
			check_and_fclose(tmplog, source);
		}
		check_and_fclose(log, destination);
	}
}

static void
copy_file(const char* source, const char* destination)
{
	char buf[4096];
	int bytes_in, bytes_out;
	int src_len = 0;
	int dst_len = 0;
	FILE *dst = fopen(destination, "wb");
	if (dst == NULL) {
		LOGE("Can't open %s\n", destination);
	} else {
		FILE *src = fopen(source, "rb");
		if (src != NULL) {
			fseek(src, 0, SEEK_SET);  // Since last write
			while ((bytes_in = fread(buf, 1, 4096, src)) > 0 ) {
				src_len += bytes_in;
				bytes_out = fwrite (buf, 1, bytes_in, dst);
				dst_len += bytes_out;
			}
			LOGE("Copy %d, Copied %d\n", src_len, dst_len);
			check_and_fclose(src, source);
		}
		check_and_fclose(dst, destination);
	}
}


// clear the recovery command and prepare to boot a (hopefully working) system,
// copy our log file to cache as well (for the system to read), and
// record any intent we were asked to communicate back to the system.
// this function is idempotent: call it as many times as you like.
static void
finish_recovery(const char *send_intent)
{
	// By this point, we're ready to return to the main system...
	if (send_intent != NULL) {
		FILE *fp = fopen_path(INTENT_FILE, "w");
		if (fp == NULL) {
			LOGE("Can't open %s\n", INTENT_FILE);
		} else {
			fputs(send_intent, fp);
			check_and_fclose(fp, INTENT_FILE);
		}
	}

	// Copy logs to cache so the system can find out what happened.
	copy_log_file(TEMPORARY_LOG_FILE, LOG_FILE, true);
	copy_log_file(TEMPORARY_LOG_FILE, LAST_LOG_FILE, false);
	copy_log_file(TEMPORARY_INSTALL_FILE, LAST_INSTALL_FILE, false);
	chmod(LOG_FILE, 0600);
	chown(LOG_FILE, 1000, 1000);   // system user
	chmod(LAST_LOG_FILE, 0640);
	chmod(LAST_INSTALL_FILE, 0644);

	// Reset to mormal system boot so recovery won't cycle indefinitely.
	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
	set_bootloader_message(&boot);

	// Remove the command file, so recovery won't repeat indefinitely.
	if (ensure_path_mounted(COMMAND_FILE) != 0 ||
	    (unlink(COMMAND_FILE) && errno != ENOENT)) {
		LOGW("Can't unlink %s\n", COMMAND_FILE);
	}

	ensure_path_unmounted(CACHE_ROOT);
	sync();  // For good measure.
}

static int
erase_volume(const char *volume)
{
	ui_set_background(BACKGROUND_ICON_INSTALLING);
	//ui_show_indeterminate_progress();
	//ui_print("Formatting %s...\n", volume);

	ensure_path_unmounted(volume);

	if (strcmp(volume, "/cache") == 0) {
		// Any part of the log we'd copied to cache is now gone.
		// Reset the pointer so we copy from the beginning of the temp
		// log.
		tmplog_offset = 0;
	}

	return format_volume(volume);
}

static char*
copy_sideloaded_package(const char* original_path)
{
	if (ensure_path_mounted(original_path) != 0) {
		LOGE("Can't mount %s\n", original_path);
		return NULL;
	}

	if (ensure_path_mounted(SIDELOAD_TEMP_DIR) != 0) {
		LOGE("Can't mount %s\n", SIDELOAD_TEMP_DIR);
		return NULL;
	}

	if (mkdir(SIDELOAD_TEMP_DIR, 0700) != 0) {
		if (errno != EEXIST) {
			LOGE("Can't mkdir %s (%s)\n", SIDELOAD_TEMP_DIR, strerror(errno));
			return NULL;
		}
	}

	// verify that SIDELOAD_TEMP_DIR is exactly what we expect: a
	// directory, owned by root, readable and writable only by root.
	struct stat st;
	if (stat(SIDELOAD_TEMP_DIR, &st) != 0) {
		LOGE("failed to stat %s (%s)\n", SIDELOAD_TEMP_DIR, strerror(errno));
		return NULL;
	}
	if (!S_ISDIR(st.st_mode)) {
		LOGE("%s isn't a directory\n", SIDELOAD_TEMP_DIR);
		return NULL;
	}
	if ((st.st_mode & 0777) != 0700) {
		LOGE("%s has perms %o\n", SIDELOAD_TEMP_DIR, st.st_mode);
		return NULL;
	}
	if (st.st_uid != 0) {
		LOGE("%s owned by %lu; not root\n", SIDELOAD_TEMP_DIR, st.st_uid);
		return NULL;
	}

	char copy_path[PATH_MAX];
	strcpy(copy_path, SIDELOAD_TEMP_DIR);
	strcat(copy_path, "/package.zip");

	char* buffer = malloc(BUFSIZ);
	if (buffer == NULL) {
		LOGE("Failed to allocate buffer\n");
		return NULL;
	}

	size_t read;
	FILE* fin = fopen(original_path, "rb");
	if (fin == NULL) {
		LOGE("Failed to open %s (%s)\n", original_path, strerror(errno));
		return NULL;
	}
	FILE* fout = fopen(copy_path, "wb");
	if (fout == NULL) {
		LOGE("Failed to open %s (%s)\n", copy_path, strerror(errno));
		return NULL;
	}

	while ((read = fread(buffer, 1, BUFSIZ, fin)) > 0) {
		if (fwrite(buffer, 1, read, fout) != read) {
			LOGE("Short write of %s (%s)\n", copy_path, strerror(errno));
			return NULL;
		}
	}

	free(buffer);

	if (fclose(fout) != 0) {
		LOGE("Failed to close %s (%s)\n", copy_path, strerror(errno));
		return NULL;
	}

	if (fclose(fin) != 0) {
		LOGE("Failed to close %s (%s)\n", original_path, strerror(errno));
		return NULL;
	}

	// "adb push" is happy to overwrite read-only files when it's
	// running as root, but we'll try anyway.
	if (chmod(copy_path, 0400) != 0) {
		LOGE("Failed to chmod %s (%s)\n", copy_path, strerror(errno));
		return NULL;
	}

	return strdup(copy_path);
}

static char**
prepend_title(const char** headers)
{
	char* title[] = { "Android system recovery <"
	                  EXPAND(RECOVERY_API_VERSION) "e>",
	                  "",
	                  NULL
	                };

	// count the number of lines in our title, plus the
	// caller-provided headers.
	int count = 0;
	char** p;
	for (p = title; *p; ++p, ++count);
	for (p = headers; *p; ++p, ++count);

	char** new_headers = malloc((count+1) * sizeof(char*));
	char** h = new_headers;
	for (p = title; *p; ++p, ++h) *h = *p;
	for (p = headers; *p; ++p, ++h) *h = *p;
	*h = NULL;

	return new_headers;
}

static int
get_menu_selection(char** headers, char** items, int menu_only,
                   int initial_selection)
{
	// throw away keys pressed previously, so user doesn't
	// accidentally trigger menu items.
	ui_clear_key_queue();

	ui_start_menu(headers, items, initial_selection);
	int selected = initial_selection;
	int chosen_item = -1;

	while (chosen_item < 0) {
		int key = ui_wait_key();
		int visible = ui_text_visible();

		if (key == -1) {   // ui_wait_key() timed out
			if (ui_text_ever_visible()) {
				continue;
			} else {
				LOGI("timed out waiting for key input; rebooting.\n");
				ui_end_menu();
				return ITEM_REBOOT;
			}
		}

		int action = device_handle_key(key, visible);

		if (action < 0) {
			switch (action) {
			case HIGHLIGHT_UP:
				--selected;
				selected = ui_menu_select(selected);
				break;
			case HIGHLIGHT_DOWN:
				++selected;
				selected = ui_menu_select(selected);
				break;
			case SELECT_ITEM:
				chosen_item = selected;
				break;
			case NO_ACTION:
				break;
			}
		} else if (!menu_only) {
			chosen_item = action;
		}
	}

	ui_end_menu();
	return chosen_item;
}

static int compare_string(const void* a, const void* b)
{
	return strcmp(*(const char**)a, *(const char**)b);
}

static int
update_directory(const char* path, const char* unmount_when_done,
                 int* wipe_cache)
{
	ensure_path_mounted(path);

	const char* MENU_HEADERS[] = { "Choose a package to install:",
	                               path,
	                               "",
	                               NULL
	                             };
	DIR* d = NULL;
	struct dirent* de;
	d = opendir(path);
	if (d == NULL) {
		LOGE("error opening %s: %s\n", path, strerror(errno));
		if (unmount_when_done != NULL) {
			ensure_path_unmounted(unmount_when_done);
		}
		return 0;
	}

	char** headers = prepend_title(MENU_HEADERS);

	int d_size = 0;
	int d_alloc = 10;
	char** dirs = malloc(d_alloc * sizeof(char*));
	int z_size = 1;
	int z_alloc = 10;
	char** zips = malloc(z_alloc * sizeof(char*));
	zips[0] = strdup("../");

	while ((de = readdir(d)) != NULL) {
		int name_len = strlen(de->d_name);

		if (de->d_type == DT_DIR) {
			// skip "." and ".." entries
			if (name_len == 1 && de->d_name[0] == '.') continue;
			if (name_len == 2 && de->d_name[0] == '.' &&
			    de->d_name[1] == '.') continue;

			if (d_size >= d_alloc) {
				d_alloc *= 2;
				dirs = realloc(dirs, d_alloc * sizeof(char*));
			}
			dirs[d_size] = malloc(name_len + 2);
			strcpy(dirs[d_size], de->d_name);
			dirs[d_size][name_len] = '/';
			dirs[d_size][name_len+1] = '\0';
			++d_size;
		} else if (de->d_type == DT_REG &&
		           name_len >= 4 &&
		           strncasecmp(de->d_name + (name_len-4), ".zip", 4) == 0) {
			if (z_size >= z_alloc) {
				z_alloc *= 2;
				zips = realloc(zips, z_alloc * sizeof(char*));
			}
			zips[z_size++] = strdup(de->d_name);
		}
	}
	closedir(d);

	qsort(dirs, d_size, sizeof(char*), compare_string);
	qsort(zips, z_size, sizeof(char*), compare_string);

	// append dirs to the zips list
	if (d_size + z_size + 1 > z_alloc) {
		z_alloc = d_size + z_size + 1;
		zips = realloc(zips, z_alloc * sizeof(char*));
	}
	memcpy(zips + z_size, dirs, d_size * sizeof(char*));
	free(dirs);
	z_size += d_size;
	zips[z_size] = NULL;

	int result;
	int chosen_item = 0;
	do {
		chosen_item = get_menu_selection(headers, zips, 1, chosen_item);

		char* item = zips[chosen_item];
		int item_len = strlen(item);
		if (chosen_item == 0) {          // item 0 is always "../"
			// go up but continue browsing (if the caller is update_directory)
			result = -1;
			break;
		} else if (item[item_len-1] == '/') {
			// recurse down into a subdirectory
			char new_path[PATH_MAX];
			strlcpy(new_path, path, PATH_MAX);
			strlcat(new_path, "/", PATH_MAX);
			strlcat(new_path, item, PATH_MAX);
			new_path[strlen(new_path)-1] = '\0';  // truncate the trailing '/'
			result = update_directory(new_path, unmount_when_done, wipe_cache);
			if (result >= 0) break;
		} else {
			// selected a zip file:  attempt to install it, and return
			// the status to the caller.
			char new_path[PATH_MAX];
			strlcpy(new_path, path, PATH_MAX);
			strlcat(new_path, "/", PATH_MAX);
			strlcat(new_path, item, PATH_MAX);

			//ui_print("\n-- Install %s ...\n", path);
			set_sdcard_update_bootloader_message();
			char* copy = copy_sideloaded_package(new_path);
			if (unmount_when_done != NULL) {
				ensure_path_unmounted(unmount_when_done);
			}
			if (copy) {
				result = install_package(copy, wipe_cache, TEMPORARY_INSTALL_FILE);
				free(copy);
			} else {
				result = INSTALL_ERROR;
			}
			break;
		}
	} while (true);

	int i;
	for (i = 0; i < z_size; ++i) free(zips[i]);
	free(zips);
	free(headers);

	if (unmount_when_done != NULL) {
		ensure_path_unmounted(unmount_when_done);
	}
	return result;
}

static void
wipe_data(int confirm)
{
	if (confirm) {
		static char** title_headers = NULL;

		if (title_headers == NULL) {
			char* headers[] = { "Confirm wipe of all user data?",
			                    "  THIS CAN NOT BE UNDONE.",
			                    "",
			                    NULL
			                  };
			title_headers = prepend_title((const char**)headers);
		}

		char* items[] = { " No",
		                  " No",
		                  " No",
		                  " No",
		                  " No",
		                  " No",
		                  " No",
		                  " Yes -- delete all user data",   // [7]
		                  " No",
		                  " No",
		                  " No",
		                  NULL
		                };

		int chosen_item = get_menu_selection(title_headers, items, 1, 0);
		if (chosen_item != 7) {
			return;
		}
	}

	//ui_print("\n-- Wiping data...\n");
	device_wipe_data();
	erase_volume("/data");
	erase_volume("/cache");
	//ui_print("Data wipe complete.\n");
}

static void
prompt_and_wait()
{
	char** headers = prepend_title((const char**)MENU_HEADERS);

	for (;;) {
		finish_recovery(NULL);
		ui_reset_progress();

		int chosen_item = get_menu_selection(headers, MENU_ITEMS, 0, 0);

		// device-specific code may take some action here.  It may
		// return one of the core actions handled in the switch
		// statement below.
		chosen_item = device_perform_action(chosen_item);

		int status;
		int wipe_cache;
		switch (chosen_item) {
		case ITEM_REBOOT:
			return;

		case ITEM_WIPE_DATA:
			wipe_data(ui_text_visible());
			if (!ui_text_visible()) return;
			break;

		case ITEM_WIPE_CACHE:
			//ui_print("\n-- Wiping cache...\n");
			erase_volume("/cache");
			//ui_print("Cache wipe complete.\n");
			if (!ui_text_visible()) return;
			break;

		case ITEM_APPLY_SDCARD:
			status = update_directory(SDCARD_ROOT, SDCARD_ROOT, &wipe_cache);
			if (status == INSTALL_SUCCESS && wipe_cache) {
				//ui_print("\n-- Wiping cache (at package request)...\n");
				if (erase_volume("/cache")) {
					//ui_print("Cache wipe failed.\n");
				} else {
					//ui_print("Cache wipe complete.\n");
				}
			}
			if (status >= 0) {
				if (status != INSTALL_SUCCESS) {
					ui_set_background(BACKGROUND_ICON_ERROR);
					//ui_print("Installation aborted.\n");
				} else if (!ui_text_visible()) {
					return;  // reboot if logs aren't visible
				} else {
					//ui_print("\nInstall from sdcard complete.\n");
				}
			}
			break;
		case ITEM_APPLY_CACHE:
			// Don't unmount cache at the end of this.
			status = update_directory(CACHE_ROOT, NULL, &wipe_cache);
			if (status == INSTALL_SUCCESS && wipe_cache) {
				//ui_print("\n-- Wiping cache (at package request)...\n");
				if (erase_volume("/cache")) {
					//ui_print("Cache wipe failed.\n");
				} else {
					//ui_print("Cache wipe complete.\n");
				}
			}
			if (status >= 0) {
				if (status != INSTALL_SUCCESS) {
					ui_set_background(BACKGROUND_ICON_ERROR);
					//ui_print("Installation aborted.\n");
				} else if (!ui_text_visible()) {
					return;  // reboot if logs aren't visible
				} else {
					//ui_print("\nInstall from cache complete.\n");
				}
			}
			break;

		}
	}
}

static void
print_property(const char *key, const char *name, void *cookie)
{
	printf("%s=%s\n", key, name);
}

static char *sirfsoc_get_os_type()
{
	char *p, *os_type;
	char cmdline_str[BUFSIZ];
	FILE *cmdline_fp;

	cmdline_fp = fopen("/proc/cmdline", "r");
	if (cmdline_fp == NULL) {
		return NULL;
	}
	fread(cmdline_str, 1, BUFSIZ, cmdline_fp);

	p = strstr(cmdline_str, "os_type");
	while (*p++ != '=');
	os_type = p;
	while ((*p != ' ') && (*p != '\0')) p++;
	*p = '\0';
	return os_type;
}

#define RFLAG_ADDR	0x0027f000 /* 2.5MB-4096 */
#define RFLAG_START	0x5a
#define RFLAG_FINISH	0xa5
static void set_flag(const char* target_media,
                     int offset, int flag)
{
	FILE* f = NULL;
	if (target_media == NULL)
		return;
	f = fopen(target_media, "wb");
	if (f == NULL) {
		fprintf(stderr, "[SetFlag]can't open %s: %s\n",
		        target_media, strerror(errno));
		return;
	}
	if (!(fseek(f, offset, SEEK_SET))) {
		fprintf(stderr, "[SetFlag]write FLAG %#x at %#x\n", flag, offset);
		putc(flag, f);
	} else {
		fprintf(stderr, "[SetFlag]fseek to position: %#x failed\n", offset);
	}
	fflush(f);
	fsync(fileno(f));
	fclose(f);
	f = NULL;
}

static int get_flag(const char* target_media, int offset)
{
	FILE* f = NULL;
	int flag = 0;
	if (target_media == NULL)
		return 0;
	f = fopen(target_media, "rb");
	if (f == NULL) {
		fprintf(stderr, "[SetFlag]can't open %s: %s\n",
		        target_media, strerror(errno));
		return 0;
	}
	if (!(fseek(f, offset, SEEK_SET))) {
		flag = getc(f);
		fprintf(stderr, "[GetFlag]read FLAG %#x at %#x\n", flag, offset);
	} else {
		fprintf(stderr, "[GetFlag]fseek to position: %#x failed\n", offset);
	}
	fclose(f);
	f = NULL;
	return flag;
}

static bool mkdirs(char *dirs)
{
	int i,len;
	char str[512];
	strncpy(str, dirs, 512);
	len=strlen(str);
	for ( i=0; i<len; i++ ) {
		if ( str[i]=='/' ) {
			str[i] = '\0';
			if ( access(str,0)!=0 ) {
				if (mkdir( str, 0777) < 0)
					return 0;
			}
			str[i]='/';
		}
	}
	if ( len>0 && access(str,0)!=0 ) {
		if (mkdir( str, 0777) < 0)
			return 0;
	}

	return 1;
}

static void set_oemdata(const char* target_media, oemdata_t* data)
{
	FILE* f = NULL;
	if (target_media == NULL)
		return;
	f = fopen(target_media, "wb");
	if (f == NULL) {
		fprintf(stderr, "OS:ERR:%s can't open %s: %s\n",
		        __FUNCTION__, target_media, strerror(errno));
		return;
	}
	if (!(fseek(f, OEMDATA_OFFSET, SEEK_SET))) {
		fprintf(stderr, "OS:MSG:OEMDATA:write at %#x:flag = %08x, magic = %08x\n", OEMDATA_OFFSET,data->update.flag, data->magic);
		fwrite(data, 1, sizeof(oemdata_t), f);
	} else {
		fprintf(stderr, "OS:ERR: fseek to position: %#x failed\n", OEMDATA_OFFSET);
	}
	fflush(f);
	fsync(fileno(f));
	fclose(f);
	f = NULL;
}

static void get_oemdata(const char* target_media, oemdata_t* data)
{
	FILE* f = NULL;
	if (target_media == NULL)
		return ;
	f = fopen(target_media, "rb");
	if (f == NULL) {
		fprintf(stderr, "OS:ERR:%s can't open %s: %s\n",
		        __FUNCTION__, target_media, strerror(errno));
		return ;
	}
	if (!(fseek(f, OEMDATA_OFFSET, SEEK_SET))) {
		fread(data, 1, sizeof(oemdata_t), f);
		fprintf(stderr, "OS:MSG:OEMDATA:read at %#x:flag = %08x, magic = %08x\n", OEMDATA_OFFSET,data->update.flag, data->magic);
	} else {
		fprintf(stderr, "OS:ERR: fseek to position: %#x failed\n", OEMDATA_OFFSET);
	}
	fclose(f);
	f = NULL;
}

#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

void set_pwm_gpio_high()
{
     	int dev_fd;

    	dev_fd = open("/dev/mem", O_RDWR | O_SYNC);

    	if (dev_fd < 0)
    	{
        	printf("open(/dev/mem) failed.");
        	return 0;
    	}	

    	unsigned char *map_base=(unsigned char * )mmap(NULL, 0x4000000, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd,  0x10000000 );

	/*set SW_TOP_FUNC_SEL_16_REG_CLR[14,12] to gpio_3*/
        *(volatile unsigned int *)(map_base + 0xE40104) |= 0x7000;
	/*set gpio_3 to PULL-UP, pull register of gpio_3 is 0x10E40254*/
        *(volatile unsigned int *)(map_base + 0XE40254) |= (0x1 << 6);
	/*set gpio_3 to HIGH, HIGH:0x60,LOW:0x20*/
        *(volatile unsigned int *)(map_base + 0x330000C) |= 0x60;
 
        munmap(map_base, 0x4000000);
}

int
main(int argc, char **argv)
{
	time_t start = time(NULL);
	FILE *serial_fp = NULL;
	serial_fp = fopen("/dev/ttySiRF1", "w");
	int ret = 0;
	int user_partno = 0;

	/*watchdog_flag
	*0: OS determines PA8 register is 0x11223344;
	*	yes:open watchdog
	*1: Close watchdog */
	char watchdog_flag[1] = "0";
	int watchdog_file = 0;

	oemdata_t oemdata;
	char* path;

	if (serial_fp != NULL) fprintf(serial_fp, "Starting recovery on %s", ctime(&start));
	// If these fail, there's not really anywhere to complain...
	freopen(TEMPORARY_LOG_FILE, "a", stdout);
	setbuf(stdout, NULL);
	freopen(TEMPORARY_LOG_FILE, "a", stderr);
	setbuf(stderr, NULL);
	printf("Starting recovery on %s", ctime(&start));

	device_ui_init(&ui_parameters);
	ui_init();
	ui_set_background(BACKGROUND_ICON_INSTALLING);
	load_volume_table();

	// mount user partition /
	bool is_factory_production = false;
	bool sda_format_err = false;
	const char *target_media = NULL;
	char user_part_nand[16], user_part_sd[16], ext_user_part_sd[16];
	char user_part[16];
	/*
	char *os_type = sirfsoc_get_os_type();
	if (!strncmp(os_type, "linux", 5)) user_partno = 6;
	if (!strncmp(os_type, "android", 8)) user_partno = 8;
	*/
	user_partno = 10;
	sprintf(user_part_nand, "/dev/nandblk0p%d", user_partno);
	sprintf(user_part_sd, "/dev/mmcblk0p%d", user_partno);
	sprintf(ext_user_part_sd, "/dev/mmcblk1p1");
	FILE* f = NULL;
	usleep(5000000);
	if ((f = fopen("/dev/sda", "rb")) != NULL) {
		sda_format_err = true;
		LOGE("/dev/sda(UDISK) exists\n");
		fclose(f);
		f = NULL;
		is_factory_production = true;
		if (mount("/dev/sda1", "/sdcard", "vfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
			fprintf(serial_fp,"mount /dev/sda1 as vfat failed\n");
			if (mount("/dev/sda1", "/sdcard", "ntfs", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
				fprintf(serial_fp,"mount /dev/sda1 as ntfs failed\n");
				if (mount("/dev/sda1", "/sdcard", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0){
					fprintf(serial_fp,"mount sda1 as ext4 failed, try to mount it as vfat ...\n");
					if (mount("/dev/sda1", "/sdcard", "texfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
						fprintf(serial_fp,"mount /dev/sda1 as texfat failed\n");
						if (mount("/dev/sda", "/sdcard", "vfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
							fprintf(serial_fp,"mount /dev/sda as vfat failed\n");
							if (mount("/dev/sda", "/sdcard", "ntfs", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
								fprintf(serial_fp,"mount /dev/sda as ntfs failed\n");
								if (mount("/dev/sda", "/sdcard", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0){
									fprintf(serial_fp,"mount sda as ext4 failed\n");
									sda_format_err = false;
									is_factory_production = false;
									fprintf(serial_fp,"The U disk format is not supported.\n");
									ui_show_progress(0.99,58);
									ui_show_text(1);
									ui_print("U disk is not supported. Upgrade using backup area file.\n");
									usleep(3000000);
									fprintf(serial_fp,"Upgrade from the backup area.\n");
								}
							}
						}
					}
				}
			}
		}
	}

	if (sda_format_err) {
		if ((f = fopen(UPDATE_PACKAGE_PATH, "rb")) == NULL) {
			fprintf(serial_fp,"%s does not exist\n", UPDATE_PACKAGE_PATH);
			umount("/sdcard");
			is_factory_production = false;
			if ((f = fopen(user_part_nand, "rb")) != NULL) {
				if (mount(user_part_nand, "/sdcard", "vfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
					fprintf(serial_fp,"mount %s failed\n", user_part_nand);
				fclose(f);
				target_media = "/dev/nandblk0";
			} else if ((f = fopen(user_part_sd, "rb")) != NULL) {
				if (mount(user_part_sd, "/sdcard", "vfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
					fprintf(serial_fp,"mount %s failed\n", user_part_sd);
				fclose(f);
				target_media = "/dev/mmcblk0";
			}
		}
		if ((f = fopen("/dev/nandblk0", "rb")) != NULL) {
			/* nand is inserted */
			/* boot media is udisk, and nand in slot0 will be updated*/
			target_media = "/dev/nandblk0";
			fclose(f);
		} else {
			target_media = "/dev/mmcblk0";
		}
		fprintf(serial_fp, "Recovery from UDISK to %s, %s", target_media, ctime(&start));
	} else if ((f = fopen(ext_user_part_sd, "rb")) != NULL) {
		/* updated from sd2 to sd0*/
		is_factory_production = true;
		target_media = "/dev/mmcblk0";
		fclose(f);
		f = NULL;
		if (mount(ext_user_part_sd, "/sdcard", "vfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
			ui_print("mount %s failed\n", ext_user_part_sd);
		fprintf(serial_fp, "Recovery from external SD to %s, %s", target_media, ctime(&start));
	} else if ((f = fopen(user_part_sd, "rb")) != NULL) {
		/* sd boot and updated from its own user partition */
		target_media = "/dev/mmcblk0";
		fclose(f);
		f = NULL;
		if (mount(user_part_sd, "/sdcard", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
			ui_print("mount %s failed\n", user_part_sd);
		fprintf(serial_fp, "Recovery from internal user partition of SD, %s", ctime(&start));
	} else {
		/* boot media is nand */
		target_media = "/dev/nandblk0";
		if (mount(user_part_nand, "/sdcard", "vfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
			ui_print("mount %s failed\n", user_part_nand);
		if ((f = fopen("/dev/mmcblk0p1", "rb")) != NULL) {
			/* sd card is inserted to update nand */
			is_factory_production = true;
			fclose(f);
			f = NULL;
			umount("/sdcard");
			if (mount("/dev/mmcblk0p1", "/sdcard", "vfat", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
				ui_print("mount /dev/mmcblk0p1 failed\n");
			fprintf(serial_fp, "Recovery from external SD to NAND, %s", ctime(&start));
		}/* else there is no sd card inserted */
		fprintf(serial_fp, "Recovery to NAND, %s", ctime(&start));
	}

	mkdir("/sdcard/cache", 755);
	ret = symlink("sdcard/cache", "/cache");
	if (ret)
		ui_print("symlink sdcard/cache -> /cache failed\n");

	// if (is_factory_production) ui_show_text(1);

	get_args(&argc, &argv);

	int previous_runs = 0;
	const char *send_intent = NULL;
	const char *update_package = NULL;
	int wipe_data = 0, wipe_cache = 0;
	const char *wifi_bt_address_path = NULL;

	int arg;
	while ((arg = getopt_long(argc, argv, "", OPTIONS, NULL)) != -1) {
		switch (arg) {
		case 'p':
			previous_runs = atoi(optarg);
			break;
		case 's':
			send_intent = optarg;
			break;
		case 'u':
			update_package = optarg;
			break;
		case 'w':
			wipe_data = wipe_cache = 1;
			break;
		case 'c':
			wipe_cache = 1;
			break;
		case 't':
			ui_show_text(1);
			break;
		case '?':
			LOGE("Invalid command argument\n");
			continue;
		}
	}

	/*
		fprintf(serial_fp, "OS:MSG:getopt_long previous_runs = %d\n", previous_runs);
		fprintf(serial_fp, "OS:MSG:getopt_long send_intent %s\n", send_intent ?  send_intent : "null" );
		fprintf(serial_fp, "OS:MSG:getopt_long update_package %s\n", update_package ? update_package :"null"  );
	*/
	device_recovery_start();

        set_pwm_gpio_high();

	printf("Command:");
	for (arg = 0; arg < argc; arg++) {
		printf(" \"%s\"", argv[arg]);
	}
	printf("\n");

	if (update_package) {
		// For backwards compatibility on the cache partition only, if
		// we're given an old 'root' path "CACHE:foo", change it to
		// "/cache/foo".
		if (strncmp(update_package, "CACHE:", 6) == 0) {
			int len = strlen(update_package) + 10;
			char* modified_path = malloc(len);
			strlcpy(modified_path, "/cache/", len);
			strlcat(modified_path, update_package+6, len);
			printf("(replacing path \"%s\" with \"%s\")\n",
			       update_package, modified_path);
			update_package = modified_path;
		}
		if (!(strcmp(update_package, "/data/card/update.zip")))
			update_package = UPDATE_PACKAGE_PATH_EXT;
		if (!(strcmp(update_package, "/mnt/sdcard/update.zip")))
			update_package = UPDATE_PACKAGE_PATH;
	}
	printf("\n");

	property_list(print_property, NULL);
	printf("\n");

	int status = INSTALL_SUCCESS;

	get_oemdata(target_media, &oemdata);

	/* when run command, update all.*/
	if ((oemdata.magic != UPDATE_MAGIC) || (oemdata.update.flag == 0)) {
		oemdata.update.flag = 0xffffffff;
	}

	set_flag(target_media, RFLAG_ADDR, RFLAG_START);
	fprintf(serial_fp, "OS:MSG:Set recovery start flag, read=%#x update_package(%s)\n",
	        get_flag(target_media, RFLAG_ADDR), update_package);

	if (update_package != NULL) {
			fprintf(serial_fp,"update_package is not null...\n");
		status = install_package(update_package, &wipe_cache, TEMPORARY_INSTALL_FILE);
		if (status == INSTALL_SUCCESS && wipe_cache) {
			if (erase_volume("/cache")) {
				LOGE("Cache wipe (requested by package) failed.");
			}
		}
		if (status != INSTALL_SUCCESS) ui_print("Installation aborted.\n");
	} else if (wipe_data) {
			fprintf(serial_fp,"wipe data...\n");
		if (device_wipe_data()) status = INSTALL_ERROR;
		if (erase_volume("/data")) status = INSTALL_ERROR;
		//if (wipe_cache && erase_volume("/cache")) status = INSTALL_ERROR;
		if (status != INSTALL_SUCCESS) {
			ui_show_text(1);
			ui_print("Data wipe failed.\n");
		}
	} else if (wipe_cache) {
			fprintf(serial_fp,"wipe cache...\n");
		if (wipe_cache && erase_volume("/cache")) status = INSTALL_ERROR;
		if (status != INSTALL_SUCCESS) ui_print("Cache wipe failed.\n");
	} else {

		if (oemdata.update.os) {
			//fprintf(serial_fp, "OS:MSG: os update ?\n");
			//status = INSTALL_ERROR;  // No command specified
			// No command specified
			// do default operation: install update.zip from /sdcard/card or /sdcard
			//ui_print("[recovery]load from /sdcard/card/update.zip\n");
			fprintf(serial_fp,"update operation system...\n");
			ui_show_progress(0.01, 1);
			ui_show_text(1);
			update_package = UPDATE_PACKAGE_PATH_EXT;
			status = install_package(UPDATE_PACKAGE_PATH_EXT, &wipe_cache, TEMPORARY_INSTALL_FILE);
			if (status != INSTALL_SUCCESS) {
				//ui_print("[recovery]no invalid update.zip in /sdcard/card/\n");
				//ui_print("[recovery]try /sdcard/update.zip\n");
				update_package = UPDATE_PACKAGE_PATH;
				status = install_package(UPDATE_PACKAGE_PATH, &wipe_cache, TEMPORARY_INSTALL_FILE);
			}
			if (status != INSTALL_SUCCESS) {
				//ui_print("[recovery]no valid update.zip\n");
				//ui_print("[recovery]prepare update.zip in external SD or internal user partition!\n");
				//ui_print("[recovery]Then retry!\n");
			}
			if (status == INSTALL_SUCCESS && wipe_cache) {
				if (erase_volume("/cache")) {
					LOGE("Cache wipe (requested by package) failed.");
				}
			}

		}
		/*mount update disk*/

		mkdir("/user", 0777);
		mkdir("/svp", 0777);
		mkdir("/storage", 0777);
		
		if(oemdata.update.svp)
		{
			fprintf(serial_fp, "OS:MSG:erase_volume svp =====\n");
			erase_volume("/svp");	
			fprintf(serial_fp, "OS:MSG:erase_volume svp down =====\n");

			fprintf(serial_fp, "OS:MSG:erase_volume storage =====\n");
			erase_volume("/storage");	
			fprintf(serial_fp, "OS:MSG:erase_volume storage down =====\n");

			fprintf(serial_fp, "OS:MSG:mount /dev/mmcblk0p5 to /svp\n");
			if(mount("/dev/mmcblk0p5", "/svp", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
			{	
				fprintf(serial_fp,"[recovery]mount /dev/mmcblk0p5 to /svp failed!\n");
				if (mount("/dev/mmcblk0p5", "/svp", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
					goto update_err;
			}

			fprintf(serial_fp, "OS:MSG:mount /dev/mmcblk0p6 to /storage\n");
			if (mount("/dev/mmcblk0p6", "/storage", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
			{
				fprintf(serial_fp,"[recovery]mount /dev/mmcblk0p6 to /storage failed!\n");
				if (mount("/dev/mmcblk0p6", "/storage", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0)
					goto update_err;
			}

			mkdirs("/storage/update/app-pkg");
			ui_reset_progress();
			ui_show_progress(0.99,30);
			ui_show_text(1);
			fprintf(serial_fp,"update service...\n");
			path = is_factory_production? UPDATE_PACKAGE_SVP_PATH_EXT : UPDATESVP_PATH;
			status = unzip_hmi(path, "svp", "/svp");
			if (status != INSTALL_SUCCESS)
			{
			       fprintf(serial_fp, "OS:MSG:unzip svp.bin failed, try second unzip!\n");
                            status = unzip_hmi(path, "svp", "/svp");
                            if (status != INSTALL_SUCCESS)
                            {
				    fprintf(serial_fp, "OS:MSG:unzip svp.bin failed, goto update_err\n");
				    goto update_err;
                            }
			}
			fprintf(serial_fp, "OS:MSG:unzip svp.bin done!\n");
			sync();
		}


		if (oemdata.update.app) {
			ui_reset_progress();
			ui_show_progress(0.99,20);
			ui_show_text(1);
			fprintf(serial_fp,"update App...\n");
			path = is_factory_production? UPDATE_PACKAGE_APP_PATH_EXT : UPDATEAPP_PATH;
			status = unzip_hmi(path, "app-pkg", "/storage/update/app-pkg");
			if (status != INSTALL_SUCCESS)
			{
			       fprintf(serial_fp, "OS:MSG:unzip app-pkg.bin failed, try second unzip!\n");
                            status = unzip_hmi(path, "app-pkg", "/storage/update/app-pkg");
                            if (status != INSTALL_SUCCESS)
                            {
				    fprintf(serial_fp, "OS:MSG:unzip app-pkg.bin failed, goto update_err\n");
				    goto update_err;
                            }
			}

			fprintf(serial_fp, "OS:MSG:unzip app-pkg.bin done!\n");
			sync();
		}

		/*uidq1644:recovery add watchdog function*/
		watchdog_file = open("/sys/devices/noc/noc:audiom/10dc0000.timer/recovery_wdt", O_WRONLY);
		if(watchdog_file == -1) {
			fprintf(serial_fp, "Recovery watchdog open failed\n");
		} else {
			ret = write(watchdog_file, watchdog_flag, sizeof(char));

			if(ret == -1)
				fprintf(serial_fp, "Recovery watchdog write failed\n");

			close(watchdog_file);
			sync();
		}

		/* update update.zip */
		if (status == INSTALL_SUCCESS && is_factory_production) {

			sprintf(user_part, "%sp10", target_media);

			/*uidq1644:U disk upgrade need to format the partition */
			if (oemdata.update.os && oemdata.update.svp && oemdata.update.app) {
				fprintf(serial_fp, "OS:MSG:erase_volume user[/media/var0] =====\n");
				erase_volume("/user");
				fprintf(serial_fp, "OS:MSG:erase_volume user[/media/var0] down =====\n");
			}

			fprintf(serial_fp, "OS:MSG:mount %s to /user[/media/var0]\n", user_part);
			if (mount(user_part, "/user", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
				fprintf(serial_fp,"[recovery]mount %s to /user[/media/var0] failed!\n", user_part);
				if (mount(user_part, "/user", "ext4", MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
					goto update_err;
				}
			}

			if (oemdata.update.os) {
				ui_reset_progress();
				ui_show_progress(0.99, 15);
				ui_show_text(1);
				fprintf(serial_fp,"backup os...\n");
				copy_file(update_package, UPDATEZIP_PATH);
				fprintf(serial_fp, "OS:MSG:copy update.zip done!\n");
			}

			if (oemdata.update.svp) {
				ui_reset_progress();
				ui_show_progress(0.99, 20);
				ui_show_text(1);
				ui_print("backup service...\n");
				copy_file(UPDATE_PACKAGE_SVP_PATH_EXT, UPDATESVP_PATH);
				sync();
				fprintf(serial_fp, "OS:MSG:copy svp.bin done!\n");
				usleep(1000000);
			}

			if (oemdata.update.app) {
				ui_reset_progress();
				ui_show_progress(0.99, 6);
				ui_show_text(1);
				ui_print("backup App...\n");
				copy_file(UPDATE_PACKAGE_APP_PATH_EXT, UPDATEAPP_PATH);
				fprintf(serial_fp, "OS:MSG:copy app-pkg.bin done!\n");
			}

		}

		/*
		       if we do not update OS, update_package = NULL , then exception and try it
		        again and again.
		*/
		/*
		        if (status == INSTALL_SUCCESS) {
			   ui_print("update_package=%s\n", update_package);
		            wifi_bt_address_path = (strstr(update_package, "sdcard/card") == NULL) ? "/sdcard/" : "/sdcard/card/";
			   ui_print("wifi_bt_address_path =%s\n", wifi_bt_address_path);
		            char *args[] = {"/bin/wifi_bt_files_storing.sh", target_media, wifi_bt_address_path, NULL};
		            pid_t child = fork();
		            if (child == 0) {
		                execv("/bin/wifi_bt_files_storing.sh", args);
		                fprintf(stderr, "recovery: execv failed: %s\n", strerror(errno));
		                _exit(1);
		            }
		            int storing_status;
		            waitpid(child, &storing_status, 0);
		            ui_print("wifi_bt_files_storing.sh returns %d\n", storing_status);
		            FILE *fp;
		            if ((fp = fopen("/tmp/wifi_bt_files_storing.info", "r")) != NULL) {
		                char buffer[1024];
		                while (fgets(buffer, sizeof(buffer), fp)) ui_print("%s", buffer);
		                fclose(fp);
		            } else {
		               storing_status = -1;
		            }
		            if (!!storing_status) {
		                ui_print("WIFI&BT address burning failed.\n");
		                ui_print("Please make sure WIFI&BT address burning is not necessary for this mass production mode\n");
		            } else {
		                is_factory_production = 1;
		            }
		        }
		*/
update_err:
		if (status != INSTALL_SUCCESS) {
			ui_show_text(1);
			ui_print("Installation aborted.\n");
			ui_print("Recovery failed\n");
		} else {
			ui_set_progress(1.0);
			ui_show_text(0);
			ui_print("Recovery finished\n");
		}
	}

	if (status != INSTALL_SUCCESS) ui_set_background(BACKGROUND_ICON_ERROR);
	if (status != INSTALL_SUCCESS || ui_text_visible()) {
		ui_print("You can find the details in %s after system reboot\n", LAST_LOG_FILE);
		ui_print("System will reboot after 30s ...\n");
		usleep(30000000);
	}

	if (status == INSTALL_SUCCESS) {
		oemdata.magic = UPDATE_MAGIC;
		oemdata.update.flag = 0;
		set_oemdata(target_media, &oemdata);
	}


	// Otherwise, get ready to boot the main system...
	finish_recovery(send_intent);
	sync();

	fprintf(serial_fp,"umount sdcard svp storage user\n");
       	umount("/sdcard");
       	umount("/svp");
       	umount("/storage");
       	umount("/user");

	if (wifi_bt_address_path != NULL)
		umount(wifi_bt_address_path);

	set_flag(target_media, RFLAG_ADDR, RFLAG_FINISH);
	fprintf(serial_fp, "Set recovery finish flag, read=%#x\n",
	        get_flag(target_media, RFLAG_ADDR));

	if (is_factory_production) {
		//ui_show_text(1);
		if (serial_fp != NULL) {
			if (status == INSTALL_SUCCESS)
				fprintf(serial_fp, "recovery finished\n");
			else
				fprintf(serial_fp, "recovery failed\n");
			fprintf(serial_fp, "For more log information, please check %s now\n", TEMPORARY_LOG_FILE);
		}
		fclose(serial_fp);
		serial_fp = NULL;
	} else {
		ui_print("Rebooting...\n");
	}

	ui_show_text(1);
	ui_print("recovery finished\n");
	usleep(1000000);
	if (status == INSTALL_SUCCESS) {
		ui_print("ok...reboot in 3s...\n");
	} else {
		ui_print("recovery failed\n");
	}

	usleep(3000000);

	android_reboot(ANDROID_RB_RESTART, 0, 0);
	return EXIT_SUCCESS;
}

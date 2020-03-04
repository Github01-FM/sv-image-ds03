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
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"
#include "install.h"
#include "mincrypt/rsa.h"
#include "minui/minui.h"
#include "minzip/SysUtil.h"
#include "minzip/Zip.h"
#include "mtdutils/mounts.h"
#include "mtdutils/mtdutils.h"
#include "roots.h"
#include "verifier.h"

#define ASSUMED_UPDATE_BINARY_NAME  "META-INF/com/google/android/update-binary"
#define PUBLIC_KEYS_FILE "/res/keys"

// If the package contains an update binary, extract it and run it.
static int
try_update_binary(const char *path, ZipArchive *zip, int* wipe_cache) {
    const ZipEntry* binary_entry =
            mzFindZipEntry(zip, ASSUMED_UPDATE_BINARY_NAME);
    if (binary_entry == NULL) {
        mzCloseZipArchive(zip);
        return INSTALL_CORRUPT;
    }

    char* binary = "/tmp/update_binary";
    unlink(binary);
    int fd = creat(binary, 0755);
    if (fd < 0) {
        mzCloseZipArchive(zip);
        LOGE("Can't make %s\n", binary);
        return INSTALL_ERROR;
    }
    bool ok = mzExtractZipEntryToFile(zip, binary_entry, fd);
    close(fd);
    mzCloseZipArchive(zip);

    if (!ok) {
        LOGE("Can't copy %s\n", ASSUMED_UPDATE_BINARY_NAME);
        return INSTALL_ERROR;
    }

    int pipefd[2];
    pipe(pipefd);

    // When executing the update binary contained in the package, the
    // arguments passed are:
    //
    //   - the version number for this interface
    //
    //   - an fd to which the program can write in order to update the
    //     progress bar.  The program can write single-line commands:
    //
    //        progress <frac> <secs>
    //            fill up the next <frac> part of of the progress bar
    //            over <secs> seconds.  If <secs> is zero, use
    //            set_progress commands to manually control the
    //            progress of this segment of the bar
    //
    //        set_progress <frac>
    //            <frac> should be between 0.0 and 1.0; sets the
    //            progress bar within the segment defined by the most
    //            recent progress command.
    //
    //        firmware <"hboot"|"radio"> <filename>
    //            arrange to install the contents of <filename> in the
    //            given partition on reboot.
    //
    //            (API v2: <filename> may start with "PACKAGE:" to
    //            indicate taking a file from the OTA package.)
    //
    //            (API v3: this command no longer exists.)
    //
    //        ui_print <string>
    //            display <string> on the screen.
    //
    //   - the name of the package zip file.
    //

         FILE *serial_fp = fopen("/dev/ttySiRF1", "w");

    char** args = malloc(sizeof(char*) * 5);
    args[0] = binary;
    args[1] = EXPAND(RECOVERY_API_VERSION);   // defined in Android.mk
    args[2] = malloc(10);
    sprintf(args[2], "%d", pipefd[1]);
    args[3] = (char*)path;
    args[4] = NULL;

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        execv(binary, args);
        fprintf(stdout, "E:Can't run %s (%s)\n", binary, strerror(errno));
        _exit(-1);
    }
    close(pipefd[1]);

    *wipe_cache = 0;

    char buffer[1024];
    FILE* from_child = fdopen(pipefd[0], "r");
    while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
        char* command = strtok(buffer, " \n");
        if (command == NULL) {
            continue;
        } else if (strcmp(command, "progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            char* seconds_s = strtok(NULL, " \n");

            float fraction = strtof(fraction_s, NULL);
            int seconds = strtol(seconds_s, NULL, 10);

            ui_show_progress(fraction * (1-VERIFICATION_PROGRESS_FRACTION),
                             seconds);
        } else if (strcmp(command, "set_progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            float fraction = strtof(fraction_s, NULL);
            ui_set_progress(fraction);
        } else if (strcmp(command, "ui_print") == 0) {
            char* str = strtok(NULL, "\n");
            if (str) {
                //ui_print("%s", str);
                fprintf(serial_fp, "%s",  str);
            } else {
                //ui_print("\n");
                fprintf(serial_fp, "\n");
            }
        } else if (strcmp(command, "wipe_cache") == 0) {
            *wipe_cache = 1;
        } else {
            LOGE("unknown command [%s]\n", command);
        }
    }
    fclose(from_child);

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        LOGE("Error in %s\n(Status %d)\n", path, WEXITSTATUS(status));
        return INSTALL_ERROR;
    }

    return INSTALL_SUCCESS;
}

// Reads a file containing one or more public keys as produced by
// DumpPublicKey:  this is an RSAPublicKey struct as it would appear
// as a C source literal, eg:
//
//  "{64,0xc926ad21,{1795090719,...,-695002876},{-857949815,...,1175080310}}"
//
// (Note that the braces and commas in this example are actual
// characters the parser expects to find in the file; the ellipses
// indicate more numbers omitted from this example.)
//
// The file may contain multiple keys in this format, separated by
// commas.  The last key must not be followed by a comma.
//
// Returns NULL if the file failed to parse, or if it contain zero keys.
static RSAPublicKey*
load_keys(const char* filename, int* numKeys) {
    RSAPublicKey* out = NULL;
    *numKeys = 0;

    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        LOGE("opening %s: %s\n", filename, strerror(errno));
        goto exit;
    }

    int i;
    bool done = false;
    while (!done) {
        ++*numKeys;
        out = realloc(out, *numKeys * sizeof(RSAPublicKey));
        RSAPublicKey* key = out + (*numKeys - 1);
        if (fscanf(f, " { %i , 0x%x , { %u",
                   &(key->len), &(key->n0inv), &(key->n[0])) != 3) {
            goto exit;
        }
        if (key->len != RSANUMWORDS) {
            LOGE("key length (%d) does not match expected size\n", key->len);
            goto exit;
        }
        for (i = 1; i < key->len; ++i) {
            if (fscanf(f, " , %u", &(key->n[i])) != 1) goto exit;
        }
        if (fscanf(f, " } , { %u", &(key->rr[0])) != 1) goto exit;
        for (i = 1; i < key->len; ++i) {
            if (fscanf(f, " , %u", &(key->rr[i])) != 1) goto exit;
        }
        fscanf(f, " } } ");

        // if the line ends in a comma, this file has more keys.
        switch (fgetc(f)) {
            case ',':
                // more keys to come.
                break;

            case EOF:
                done = true;
                break;

            default:
                LOGE("unexpected character between keys\n");
                goto exit;
        }
    }

    fclose(f);
    return out;

exit:
    if (f) fclose(f);
    free(out);
    *numKeys = 0;
    return NULL;
}

static int
really_install_package(const char *path, int* wipe_cache)
{
    ui_set_background(BACKGROUND_ICON_INSTALLING);
    //ui_print("Finding update package...\n");
    //ui_show_indeterminate_progress();
    LOGI("Update location: %s\n", path);

    if (ensure_path_mounted(path) != 0) {
        LOGE("Can't mount %s\n", path);
        return INSTALL_CORRUPT;
    }

    //ui_print("Opening update package...\n");

    int numKeys;
    RSAPublicKey* loadedKeys = load_keys(PUBLIC_KEYS_FILE, &numKeys);
    if (loadedKeys == NULL) {
        LOGE("Failed to load keys\n");
        return INSTALL_CORRUPT;
    }
    LOGI("%d key(s) loaded from %s\n", numKeys, PUBLIC_KEYS_FILE);

    // Give verification half the progress bar...
    //ui_print("Verifying update package...\n");
    ui_show_progress(
            VERIFICATION_PROGRESS_FRACTION,
            VERIFICATION_PROGRESS_TIME);

    int err;
    err = verify_file(path, loadedKeys, numKeys);
    free(loadedKeys);
    LOGI("verify_file returned %d\n", err);
    if (err != VERIFY_SUCCESS) {
        LOGE("signature verification failed\n");
        return INSTALL_CORRUPT;
    }

    /* Try to open the package.
     */
    ZipArchive zip;
    err = mzOpenZipArchive(path, &zip);
    if (err != 0) {
        LOGE("Can't open %s\n(%s)\n", path, err != -1 ? strerror(err) : "bad");
        return INSTALL_CORRUPT;
    }

    /* Verify and install the contents of the package.
     */
    //ui_print("Installing update...\n");
    return try_update_binary(path, &zip, wipe_cache);
}

int
install_package(const char* path, int* wipe_cache, const char* install_file)
{
    FILE* install_log = fopen_path(install_file, "w");
    if (install_log) {
        fputs(path, install_log);
        fputc('\n', install_log);
    } else {
        LOGE("failed to open last_install: %s\n", strerror(errno));
    }
    int result = really_install_package(path, wipe_cache);
    if (install_log) {
        fputc(result == INSTALL_SUCCESS ? '1' : '0', install_log);
        fputc('\n', install_log);
        fclose(install_log);
    }
    return result;
}

 
 int unzip_hmi(const char* src_path, const char* zip_dir, const char* dst_path) 
{ 
        LOGI("Update location: %s\n", src_path);


        if (ensure_path_mounted(src_path) != 0) {
        LOGE("Can't mount %s\n", src_path);
        return INSTALL_CORRUPT;
        }

        if (ensure_path_mounted(dst_path) != 0) 
        {
                LOGE("Can't mount %s\n", dst_path);
                return INSTALL_CORRUPT;
        }

        //ui_print("Opening update package...\n");

        /* Try to open the package.*/
        ZipArchive zip;
        int err = 0;
        err = mzOpenZipArchive(src_path, &zip);
        if (err != 0) 
        {
                LOGI("OS:ERR:Can't open %s\n(%s)\n", src_path, err != -1 ? strerror(err) : "bad");
                return INSTALL_CORRUPT;
        }    
        // To create a consistent system image, never use the clock for timestamps. 
        struct utimbuf timestamp = { 1217592000, 1217592000 };  // 8/1/2008 default 

        LOGI("Update location: %s  to %s\n", src_path, dst_path);

		bool success = false;
        success = mzExtractRecursive(&zip, zip_dir, dst_path,
                    MZ_EXTRACT_FILES_ONLY, &timestamp, 
                    NULL, NULL);

		dirSetHierarchyPermissions(dst_path, 0, 0, 0777, 0777);
        mzCloseZipArchive(&zip);

		LOGI("OS:MSG: update %s\n", ((success==true)?"success":"failed"));

		if(false == success)
			return INSTALL_CORRUPT;

		return INSTALL_SUCCESS;
} 
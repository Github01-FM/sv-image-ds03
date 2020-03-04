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
 * Copyright (C) 2009 The Android Open Source Project
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <linux/fs.h>
#include <mtd/mtd-user.h>

#include "cutils/misc.h"
#include "cutils/properties.h"
#include "edify/expr.h"
#include "mincrypt/sha.h"
#include "minzip/DirUtil.h"
#include "minelf/Retouch.h"
#include "mtdutils/mounts.h"
#include "mtdutils/mtdutils.h"
#include "updater.h"
#include "applypatch/applypatch.h"

#ifdef USE_EXT4
#include "make_ext4fs.h"
#endif

#define COMMIT_FLAG_ADDR 0x200000
#define RADAR_IMAGE_OFFSET (6*1024*1024)
#define NOR_FLASH_SIZE (8*1024*1024)
// mount(fs_type, partition_type, location, mount_point)
//
//    fs_type="yaffs2" partition_type="MTD"     location=partition
//    fs_type="ext4"   partition_type="EMMC"    location=device
Value* MountFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 4) {
        return ErrorAbort(state, "%s() expects 4 args, got %d", name, argc);
    }
    char* fs_type;
    char* partition_type;
    char* location;
    char* mount_point;
    if (ReadArgs(state, argv, 4, &fs_type, &partition_type,
                 &location, &mount_point) < 0) {
        return NULL;
    }

    if (strlen(fs_type) == 0) {
        ErrorAbort(state, "fs_type argument to %s() can't be empty", name);
        goto done;
    }
    if (strlen(partition_type) == 0) {
        ErrorAbort(state, "partition_type argument to %s() can't be empty",
                   name);
        goto done;
    }
    if (strlen(location) == 0) {
        ErrorAbort(state, "location argument to %s() can't be empty", name);
        goto done;
    }
    if (strlen(mount_point) == 0) {
        ErrorAbort(state, "mount_point argument to %s() can't be empty", name);
        goto done;
    }

    mkdir(mount_point, 0755);

    if (strcmp(partition_type, "MTD") == 0) {
        mtd_scan_partitions();
        const MtdPartition* mtd;
        mtd = mtd_find_partition_by_name(location);
        if (mtd == NULL) {
            fprintf(stderr, "%s: no mtd partition named \"%s\"",
                    name, location);
            result = strdup("");
            goto done;
        }
        if (mtd_mount_partition(mtd, mount_point, fs_type, 0 /* rw */) != 0) {
            fprintf(stderr, "mtd mount of %s failed: %s\n",
                    location, strerror(errno));
            result = strdup("");
            goto done;
        }
        result = mount_point;
    } else {
        if (mount(location, mount_point, fs_type,
                  MS_NOATIME | MS_NODEV | MS_NODIRATIME, "") < 0) {
            fprintf(stderr, "%s: failed to mount %s at %s: %s\n",
                    name, location, mount_point, strerror(errno));
            result = strdup("");
        } else {
            result = mount_point;
        }
    }

done:
    free(fs_type);
    free(partition_type);
    free(location);
    if (result != mount_point) free(mount_point);
    return StringValue(result);
}


// is_mounted(mount_point)
Value* IsMountedFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 1 arg, got %d", name, argc);
    }
    char* mount_point;
    if (ReadArgs(state, argv, 1, &mount_point) < 0) {
        return NULL;
    }
    if (strlen(mount_point) == 0) {
        ErrorAbort(state, "mount_point argument to unmount() can't be empty");
        goto done;
    }

    scan_mounted_volumes();
    const MountedVolume* vol = find_mounted_volume_by_mount_point(mount_point);
    if (vol == NULL) {
        result = strdup("");
    } else {
        result = mount_point;
    }

done:
    if (result != mount_point) free(mount_point);
    return StringValue(result);
}


Value* UnmountFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 1 arg, got %d", name, argc);
    }
    char* mount_point;
    if (ReadArgs(state, argv, 1, &mount_point) < 0) {
        return NULL;
    }
    if (strlen(mount_point) == 0) {
        ErrorAbort(state, "mount_point argument to unmount() can't be empty");
        goto done;
    }

    scan_mounted_volumes();
    const MountedVolume* vol = find_mounted_volume_by_mount_point(mount_point);
    if (vol == NULL) {
        fprintf(stderr, "unmount of %s failed; no such volume\n", mount_point);
        result = strdup("");
    } else {
        unmount_mounted_volume(vol);
        result = mount_point;
    }

done:
    if (result != mount_point) free(mount_point);
    return StringValue(result);
}

//format_ext4(location, volume)
Value* FormatExt4Fn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 2) {
        return ErrorAbort(state, "%s() expects 2 args, got %d", name, argc);
    }
    char* location;
    char* volume;
    if (ReadArgs(state, argv, 2, &location, &volume) < 0) {
        return NULL;
    }

    if (strlen(location) == 0) {
        ErrorAbort(state, "location argument to %s() can't be empty", name);
        goto done;
    }

    if (strlen(volume) == 0) {
        ErrorAbort(state, "location argument to %s() can't be empty", name);
        goto done;
    }
    char *args[] = {"mke2fs", "-T", "ext4", "-L", volume, location, NULL};
    pid_t child = fork();
    if (child == 0) {
        execv("/bin/mke2fs", args);
        fprintf(stderr, "format_ext4: execv failed: %s\n", strerror(errno));
        _exit(1);
    }
    int status;
    waitpid(child, &status, 0);
    if (!!status) goto done;
    result = location;

done:
    free(volume);
    if (result != location) free(location);
    return StringValue(result);

}

// format(fs_type, partition_type, location, fs_size)
//
//    fs_type="yaffs2" partition_type="MTD"     location=partition fs_size=<bytes>
//    fs_type="ext4"   partition_type="EMMC"    location=device    fs_size=<bytes>
//    if fs_size == 0, then make_ext4fs uses the entire partition.
//    if fs_size > 0, that is the size to use
//    if fs_size < 0, then reserve that many bytes at the end of the partition
Value* FormatFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 4) {
        return ErrorAbort(state, "%s() expects 4 args, got %d", name, argc);
    }
    char* fs_type;
    char* partition_type;
    char* location;
    char* fs_size;
    if (ReadArgs(state, argv, 4, &fs_type, &partition_type, &location, &fs_size) < 0) {
        return NULL;
    }

    if (strlen(fs_type) == 0) {
        ErrorAbort(state, "fs_type argument to %s() can't be empty", name);
        goto done;
    }
    if (strlen(partition_type) == 0) {
        ErrorAbort(state, "partition_type argument to %s() can't be empty",
                   name);
        goto done;
    }
    if (strlen(location) == 0) {
        ErrorAbort(state, "location argument to %s() can't be empty", name);
        goto done;
    }

    if (strcmp(partition_type, "MTD") == 0) {
        mtd_scan_partitions();
        const MtdPartition* mtd = mtd_find_partition_by_name(location);
        if (mtd == NULL) {
            fprintf(stderr, "%s: no mtd partition named \"%s\"",
                    name, location);
            result = strdup("");
            goto done;
        }
        MtdWriteContext* ctx = mtd_write_partition(mtd);
        if (ctx == NULL) {
            fprintf(stderr, "%s: can't write \"%s\"", name, location);
            result = strdup("");
            goto done;
        }
        if (mtd_erase_blocks(ctx, -1) == -1) {
            mtd_write_close(ctx);
            fprintf(stderr, "%s: failed to erase \"%s\"", name, location);
            result = strdup("");
            goto done;
        }
        if (mtd_write_close(ctx) != 0) {
            fprintf(stderr, "%s: failed to close \"%s\"", name, location);
            result = strdup("");
            goto done;
        }
        result = location;
#ifdef USE_EXT4
    } else if (strcmp(fs_type, "ext4") == 0) {
        int status = make_ext4fs(location, atoll(fs_size));
        if (status != 0) {
            fprintf(stderr, "%s: make_ext4fs failed (%d) on %s",
                    name, status, location);
            result = strdup("");
            goto done;
        }
        result = location;
#endif
    } else {
        fprintf(stderr, "%s: unsupported fs_type \"%s\" partition_type \"%s\"",
                name, fs_type, partition_type);
    }

done:
    free(fs_type);
    free(partition_type);
    if (result != location) free(location);
    return StringValue(result);
}


Value* DeleteFn(const char* name, State* state, int argc, Expr* argv[]) {
    char** paths = malloc(argc * sizeof(char*));
    int i;
    for (i = 0; i < argc; ++i) {
        paths[i] = Evaluate(state, argv[i]);
        if (paths[i] == NULL) {
            int j;
            for (j = 0; j < i; ++i) {
                free(paths[j]);
            }
            free(paths);
            return NULL;
        }
    }

    bool recursive = (strcmp(name, "delete_recursive") == 0);

    int success = 0;
    for (i = 0; i < argc; ++i) {
        if ((recursive ? dirUnlinkHierarchy(paths[i]) : unlink(paths[i])) == 0)
            ++success;
        free(paths[i]);
    }
    free(paths);

    char buffer[10];
    sprintf(buffer, "%d", success);
    return StringValue(strdup(buffer));
}


Value* ShowProgressFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc != 2) {
        return ErrorAbort(state, "%s() expects 2 args, got %d", name, argc);
    }
    char* frac_str;
    char* sec_str;
    if (ReadArgs(state, argv, 2, &frac_str, &sec_str) < 0) {
        return NULL;
    }

    double frac = strtod(frac_str, NULL);
    int sec = strtol(sec_str, NULL, 10);

    UpdaterInfo* ui = (UpdaterInfo*)(state->cookie);
    fprintf(ui->cmd_pipe, "progress %f %d\n", frac, sec);

    free(sec_str);
    return StringValue(frac_str);
}

Value* SetProgressFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 1 arg, got %d", name, argc);
    }
    char* frac_str;
    if (ReadArgs(state, argv, 1, &frac_str) < 0) {
        return NULL;
    }

    double frac = strtod(frac_str, NULL);

    UpdaterInfo* ui = (UpdaterInfo*)(state->cookie);
    fprintf(ui->cmd_pipe, "set_progress %f\n", frac);

    return StringValue(frac_str);
}

// package_extract_dir(package_path, destination_path)
Value* PackageExtractDirFn(const char* name, State* state,
                          int argc, Expr* argv[]) {
    if (argc != 2) {
        return ErrorAbort(state, "%s() expects 2 args, got %d", name, argc);
    }
    char* zip_path;
    char* dest_path;
    if (ReadArgs(state, argv, 2, &zip_path, &dest_path) < 0) return NULL;

    ZipArchive* za = ((UpdaterInfo*)(state->cookie))->package_zip;

    // To create a consistent system image, never use the clock for timestamps.
    struct utimbuf timestamp = { 1217592000, 1217592000 };  // 8/1/2008 default

    bool success = mzExtractRecursive(za, zip_path, dest_path,
                                      MZ_EXTRACT_FILES_ONLY, &timestamp,
                                      NULL, NULL);
    free(zip_path);
    free(dest_path);
    return StringValue(strdup(success ? "t" : ""));
}

// package_extract_dir(package_path, destination_path)
Value* ExtractDirFn(const char* name, State* state,
                          int argc, Expr* argv[]) {
    if (argc != 3) {
        return ErrorAbort(state, "%s() expects 3 args, got %d", name, argc);
    }
    char* zip_file_path;
    char* zip_path;
    char* dest_path;
    bool rc;
    if (ReadArgs(state, argv, 3, &zip_file_path, &zip_path, &dest_path) < 0) return NULL;

    ZipArchive* za;
    rc = mzOpenZipArchive(zip_file_path, &za); 
    fprintf(stderr, "OS:MSG:%s: mzOpenZipArchive:return value(%d)\n",name, rc);

    // To create a consistent system image, never use the clock for timestamps.
    struct utimbuf timestamp = { 1217592000, 1217592000 };  // 8/1/2008 default

    bool success = mzExtractRecursive(za, zip_path, dest_path,
                                      MZ_EXTRACT_FILES_ONLY, &timestamp,
                                      NULL, NULL);

    fprintf(stderr, "OS:MSG:%s: mzExtractRecursive:return value(%d)\n",name, success);
    free(zip_path);
    free(dest_path);
    return StringValue(strdup(success ? "t" : ""));
}


// package_extract_file(package_path, destination_path)
//   or
// package_extract_file(package_path)
//   to return the entire contents of the file as the result of this
//   function (the char* returned is actually a FileContents*).
Value* PackageExtractFileFn(const char* name, State* state,
                           int argc, Expr* argv[]) {
    if (argc != 1 && argc != 2) {
        return ErrorAbort(state, "%s() expects 1 or 2 args, got %d",
                          name, argc);
    }
    bool success = false;
    if (argc == 2) {
        // The two-argument version extracts to a file.

        char* zip_path;
        char* dest_path;
        if (ReadArgs(state, argv, 2, &zip_path, &dest_path) < 0) return NULL;

        ZipArchive* za = ((UpdaterInfo*)(state->cookie))->package_zip;
        const ZipEntry* entry = mzFindZipEntry(za, zip_path);
        if (entry == NULL) {
            fprintf(stderr, "%s: no %s in package\n", name, zip_path);
            goto done2;
        }

        FILE* f = fopen(dest_path, "wb");
        if (f == NULL) {
            fprintf(stderr, "%s: can't open %s for write: %s\n",
                    name, dest_path, strerror(errno));
            goto done2;
        }
        success = mzExtractZipEntryToFile(za, entry, fileno(f));
        fclose(f);

      done2:
        free(zip_path);
        free(dest_path);
        return StringValue(strdup(success ? "t" : ""));
    } else {
        // The one-argument version returns the contents of the file
        // as the result.

        char* zip_path;
        Value* v = malloc(sizeof(Value));
        v->type = VAL_BLOB;
        v->size = -1;
        v->data = NULL;

        if (ReadArgs(state, argv, 1, &zip_path) < 0) return NULL;

        ZipArchive* za = ((UpdaterInfo*)(state->cookie))->package_zip;
        const ZipEntry* entry = mzFindZipEntry(za, zip_path);
        if (entry == NULL) {
            fprintf(stderr, "%s: no %s in package\n", name, zip_path);
            goto done1;
        }

        v->size = mzGetZipEntryUncompLen(entry);
        v->data = malloc(v->size);
        if (v->data == NULL) {
            fprintf(stderr, "%s: failed to allocate %ld bytes for %s\n",
                    name, (long)v->size, zip_path);
            goto done1;
        }

        success = mzExtractZipEntryToBuffer(za, entry,
                                            (unsigned char *)v->data);

      done1:
        free(zip_path);
        if (!success) {
            free(v->data);
            v->data = NULL;
            v->size = -1;
        }
        return v;
    }
}


// retouch_binaries(lib1, lib2, ...)
Value* RetouchBinariesFn(const char* name, State* state,
                         int argc, Expr* argv[]) {
    UpdaterInfo* ui = (UpdaterInfo*)(state->cookie);

    char **retouch_entries  = ReadVarArgs(state, argc, argv);
    if (retouch_entries == NULL) {
        return StringValue(strdup("t"));
    }

    // some randomness from the clock
    int32_t override_base;
    bool override_set = false;
    int32_t random_base = time(NULL) % 1024;
    // some more randomness from /dev/random
    FILE *f_random = fopen("/dev/random", "rb");
    uint16_t random_bits = 0;
    if (f_random != NULL) {
        fread(&random_bits, 2, 1, f_random);
        random_bits = random_bits % 1024;
        fclose(f_random);
    }
    random_base = (random_base + random_bits) % 1024;
    fprintf(ui->cmd_pipe, "ui_print Random offset: 0x%x\n", random_base);
    fprintf(ui->cmd_pipe, "ui_print\n");

    // make sure we never randomize to zero; this let's us look at a file
    // and know for sure whether it has been processed; important in the
    // crash recovery process
    if (random_base == 0) random_base = 1;
    // make sure our randomization is page-aligned
    random_base *= -0x1000;
    override_base = random_base;

    int i = 0;
    bool success = true;
    while (i < (argc - 1)) {
        success = success && retouch_one_library(retouch_entries[i],
                                                 retouch_entries[i+1],
                                                 random_base,
                                                 override_set ?
                                                   NULL :
                                                   &override_base);
        if (!success)
            ErrorAbort(state, "Failed to retouch '%s'.", retouch_entries[i]);

        free(retouch_entries[i]);
        free(retouch_entries[i+1]);
        i += 2;

        if (success && override_base != 0) {
            random_base = override_base;
            override_set = true;
        }
    }
    if (i < argc) {
        free(retouch_entries[i]);
        success = false;
    }
    free(retouch_entries);

    if (!success) {
      Value* v = malloc(sizeof(Value));
      v->type = VAL_STRING;
      v->data = NULL;
      v->size = -1;
      return v;
    }
    return StringValue(strdup("t"));
}


// undo_retouch_binaries(lib1, lib2, ...)
Value* UndoRetouchBinariesFn(const char* name, State* state,
                             int argc, Expr* argv[]) {
    UpdaterInfo* ui = (UpdaterInfo*)(state->cookie);

    char **retouch_entries  = ReadVarArgs(state, argc, argv);
    if (retouch_entries == NULL) {
        return StringValue(strdup("t"));
    }

    int i = 0;
    bool success = true;
    int32_t override_base;
    while (i < (argc-1)) {
        success = success && retouch_one_library(retouch_entries[i],
                                                 retouch_entries[i+1],
                                                 0 /* undo => offset==0 */,
                                                 NULL);
        if (!success)
            ErrorAbort(state, "Failed to unretouch '%s'.",
                       retouch_entries[i]);

        free(retouch_entries[i]);
        free(retouch_entries[i+1]);
        i += 2;
    }
    if (i < argc) {
        free(retouch_entries[i]);
        success = false;
    }
    free(retouch_entries);

    if (!success) {
      Value* v = malloc(sizeof(Value));
      v->type = VAL_STRING;
      v->data = NULL;
      v->size = -1;
      return v;
    }
    return StringValue(strdup("t"));
}


// symlink target src1 src2 ...
//    unlinks any previously existing src1, src2, etc before creating symlinks.
Value* SymlinkFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc == 0) {
        return ErrorAbort(state, "%s() expects 1+ args, got %d", name, argc);
    }
    char* target;
    target = Evaluate(state, argv[0]);
    if (target == NULL) return NULL;

    char** srcs = ReadVarArgs(state, argc-1, argv+1);
    if (srcs == NULL) {
        free(target);
        return NULL;
    }

    int i;
    for (i = 0; i < argc-1; ++i) {
        if (unlink(srcs[i]) < 0) {
            if (errno != ENOENT) {
                fprintf(stderr, "%s: failed to remove %s: %s\n",
                        name, srcs[i], strerror(errno));
            }
        }
        if (symlink(target, srcs[i]) < 0) {
            fprintf(stderr, "%s: failed to symlink %s to %s: %s\n",
                    name, srcs[i], target, strerror(errno));
        }
        free(srcs[i]);
    }
    free(srcs);
    return StringValue(strdup(""));
}


Value* SetPermFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    bool recursive = (strcmp(name, "set_perm_recursive") == 0);

    int min_args = 4 + (recursive ? 1 : 0);
    if (argc < min_args) {
        return ErrorAbort(state, "%s() expects %d+ args, got %d", name, argc);
    }

    char** args = ReadVarArgs(state, argc, argv);
    if (args == NULL) return NULL;

    char* end;
    int i;

    int uid = strtoul(args[0], &end, 0);
    if (*end != '\0' || args[0][0] == 0) {
        ErrorAbort(state, "%s: \"%s\" not a valid uid", name, args[0]);
        goto done;
    }

    int gid = strtoul(args[1], &end, 0);
    if (*end != '\0' || args[1][0] == 0) {
        ErrorAbort(state, "%s: \"%s\" not a valid gid", name, args[1]);
        goto done;
    }

    if (recursive) {
        int dir_mode = strtoul(args[2], &end, 0);
        if (*end != '\0' || args[2][0] == 0) {
            ErrorAbort(state, "%s: \"%s\" not a valid dirmode", name, args[2]);
            goto done;
        }

        int file_mode = strtoul(args[3], &end, 0);
        if (*end != '\0' || args[3][0] == 0) {
            ErrorAbort(state, "%s: \"%s\" not a valid filemode",
                       name, args[3]);
            goto done;
        }

        for (i = 4; i < argc; ++i) {
            dirSetHierarchyPermissions(args[i], uid, gid, dir_mode, file_mode);
        }
    } else {
        int mode = strtoul(args[2], &end, 0);
        if (*end != '\0' || args[2][0] == 0) {
            ErrorAbort(state, "%s: \"%s\" not a valid mode", name, args[2]);
            goto done;
        }

        for (i = 3; i < argc; ++i) {
            if (chown(args[i], uid, gid) < 0) {
                fprintf(stderr, "%s: chown of %s to %d %d failed: %s\n",
                        name, args[i], uid, gid, strerror(errno));
            }
            if (chmod(args[i], mode) < 0) {
                fprintf(stderr, "%s: chmod of %s to %o failed: %s\n",
                        name, args[i], mode, strerror(errno));
            }
        }
    }
    result = strdup("");

done:
    for (i = 0; i < argc; ++i) {
        free(args[i]);
    }
    free(args);

    return StringValue(result);
}


Value* GetPropFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 1 arg, got %d", name, argc);
    }
    char* key;
    key = Evaluate(state, argv[0]);
    if (key == NULL) return NULL;

    char value[PROPERTY_VALUE_MAX];
    property_get(key, value, "");
    free(key);

    return StringValue(strdup(value));
}


// file_getprop(file, key)
//
//   interprets 'file' as a getprop-style file (key=value pairs, one
//   per line, # comment lines and blank lines okay), and returns the value
//   for 'key' (or "" if it isn't defined).
Value* FileGetPropFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    char* buffer = NULL;
    char* filename;
    char* key;
    if (ReadArgs(state, argv, 2, &filename, &key) < 0) {
        return NULL;
    }

    struct stat st;
    if (stat(filename, &st) < 0) {
        ErrorAbort(state, "%s: failed to stat \"%s\": %s",
                   name, filename, strerror(errno));
        goto done;
    }

#define MAX_FILE_GETPROP_SIZE    65536

    if (st.st_size > MAX_FILE_GETPROP_SIZE) {
        ErrorAbort(state, "%s too large for %s (max %d)",
                   filename, name, MAX_FILE_GETPROP_SIZE);
        goto done;
    }

    buffer = malloc(st.st_size+1);
    if (buffer == NULL) {
        ErrorAbort(state, "%s: failed to alloc %d bytes", name, st.st_size+1);
        goto done;
    }

    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        ErrorAbort(state, "%s: failed to open %s: %s",
                   name, filename, strerror(errno));
        goto done;
    }

    if (fread(buffer, 1, st.st_size, f) != st.st_size) {
        ErrorAbort(state, "%s: failed to read %d bytes from %s",
                   name, st.st_size+1, filename);
        fclose(f);
        goto done;
    }
    buffer[st.st_size] = '\0';

    fclose(f);

    char* line = strtok(buffer, "\n");
    do {
        // skip whitespace at start of line
        while (*line && isspace(*line)) ++line;

        // comment or blank line: skip to next line
        if (*line == '\0' || *line == '#') continue;

        char* equal = strchr(line, '=');
        if (equal == NULL) {
            ErrorAbort(state, "%s: malformed line \"%s\": %s not a prop file?",
                       name, line, filename);
            goto done;
        }

        // trim whitespace between key and '='
        char* key_end = equal-1;
        while (key_end > line && isspace(*key_end)) --key_end;
        key_end[1] = '\0';

        // not the key we're looking for
        if (strcmp(key, line) != 0) continue;

        // skip whitespace after the '=' to the start of the value
        char* val_start = equal+1;
        while(*val_start && isspace(*val_start)) ++val_start;

        // trim trailing whitespace
        char* val_end = val_start + strlen(val_start)-1;
        while (val_end > val_start && isspace(*val_end)) --val_end;
        val_end[1] = '\0';

        result = strdup(val_start);
        break;

    } while ((line = strtok(NULL, "\n")));

    if (result == NULL) result = strdup("");

  done:
    free(filename);
    free(key);
    free(buffer);
    return StringValue(result);
}


static bool write_raw_image_cb(const unsigned char* data,
                               int data_len, void* ctx) {
    int r = mtd_write_data((MtdWriteContext*)ctx, (const char *)data, data_len);
    if (r == data_len) return true;
    fprintf(stderr, "%s\n", strerror(errno));
    return false;
}


static int erase_flash(int fd, int offset, int bytes)
{
	int err;
	struct erase_info_user erase;
	erase.start = offset;
	erase.length = bytes;
	err = ioctl(fd, MEMERASE, &erase);
	if (err < 0) {
		perror("MEMERASE");
		return 1;
	}
	fprintf(stderr, "Erased %d bytes from address 0x%.8x in flash\n", bytes, offset);
	return 0;
}

static int file_to_flash(int fd, int offset, int len, const char *filename)
{
	u_int8_t *buf = NULL;
	FILE *fp;
	int err;
	int size = len * sizeof(u_int8_t);
	int n = len;

	if (offset != lseek(fd, offset, SEEK_SET)) {
		perror("lseek()");
		return 1;
	}
	if ((fp = fopen(filename, "r")) == NULL) {
		perror("fopen()");
		return 1;
	}
retry:
	if ((buf = (u_int8_t *) malloc(size)) == NULL) {
		fprintf(stderr, "%s: malloc(%#x) failed\n", __func__, size);
		if (size != BUFSIZ) {
			size = BUFSIZ;
			fprintf(stderr, "%s: trying buffer size %#x\n", __func__, size);
			goto retry;
		}
		perror("malloc()");
		fclose(fp);
		return 1;
	}
	do {
		if (n <= size)
			size = n;
		if (fread(buf, size, 1, fp) != 1 || ferror(fp)) {
			fprintf(stderr, "%s: fread, size %#x, n %#x\n", __func__, size, n);
			perror("fread()");
			free(buf);
			fclose(fp);
			return 1;
		}
		err = write(fd, buf, size);
		if (err < 0) {
			fprintf(stderr, "%s: write, size %#x, n %#x\n", __func__, size, n);
			perror("write()");
			free(buf);
			fclose(fp);
			return 1;
		}
		n -= size;
	} while (n > 0);

	if (buf != NULL)
		free(buf);
	fclose(fp);
	printf("Copied %d bytes from %s to address 0x%.8x in flash\n", len, filename, offset);
	return 0;
}

//write_freertos(file, partition)
Value* WriteFreertosFn(const char* name, State* state, int argc, Expr* argv[])
{

	char* result = NULL;
	int rfd, wfd;
	int rf_size = 0, err = 0;
	struct stat st;
	char* filename;
	char* partition;
	bool success = true;

	if (ReadArgs(state, argv, 2, &filename, &partition) < 0) {
		return NULL;
	}
	if (strlen(partition) == 0) {
		ErrorAbort(state, "partition argument to %s can't be empty", name);
		goto done;
	}
	if (strlen(filename) == 0) {
		ErrorAbort(state, "file argument to %s can't be empty", name);
		goto done;
	}
	
	if ((rfd = open(filename, O_SYNC | O_RDONLY)) < 0) {
		fprintf(stderr, "%s: open %s failed\n", name, filename);
        	result = strdup("");
		goto done;
	}

	if ((wfd = open(partition, O_SYNC | O_RDWR)) < 0) {
		fprintf(stderr, "%s: open %s failed\n", name, partition);
        	result = strdup("");
		goto done;
	}

	stat(filename, &st);
	rf_size = st.st_size;
	fprintf(stderr, "erase Nor flash\n");
	err = erase_flash(wfd, 0, NOR_FLASH_SIZE);
	if (err) {
		fprintf(stderr, "%s, erase SPI NOR failed\n", name);
		success = false;
		goto done;
	}
	fprintf(stderr, "%s (%d bytes) be worte\n", filename, rf_size);
	err = file_to_flash(wfd, 0, rf_size, filename);
	if (err) {
		fprintf(stderr, "%s, wirte Freertos to SPI NOR failed\n", name);
		success = false;
		goto done;
	}

	/* close device */
	if (close(rfd) < 0)
		fprintf(stderr, "close rfd failed\n");
	if (close(wfd) < 0)
		fprintf(stderr, "close wfd failed\n");


	result = success ? partition : strdup("");

done:
	if (result != partition) free(partition);
	free(filename);
	return StringValue(result);
}


Value* WriteRadarIamgeFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;

    char* partition;
    char* filename;
    bool is_emmc = false;

    fprintf(stderr, "recovery: WriteRadarIamgeFn in \n");
    if (ReadArgs(state, argv, 2, &filename, &partition) < 0) {
        return NULL;
    }

    if (strcmp(partition, "/dev/mmcblk0") == 0) is_emmc = true;

    if (strlen(partition) == 0) {
        ErrorAbort(state, "partition argument to %s can't be empty", name);
        goto done;
    }
    if (strlen(filename) == 0) {
        ErrorAbort(state, "file argument to %s can't be empty", name);
        goto done;
    }
    bool success;
    fprintf(stderr, "[WriteRadarIamgeFn] open %s\n", filename);
    FILE* f = NULL;
    f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, filename, strerror(errno));
        result = strdup("");
        goto done;
    }
    long read_position;
    read_position = 0;
    if (!!(fseek(f, read_position, SEEK_SET))) {
        fprintf(stderr, "[WriteRadarIamgeFn]fseek failed, %s\n", strerror(errno));
        result = strdup("");
        goto done;
    }
    fprintf(stderr, "[WriteRadarIamgeFn]fseek to read position: %d\n", (int)read_position);
    fprintf(stderr, "[WriteRadarIamgeFn] open %s\n", partition);
    FILE* raw_f = NULL;
    raw_f = fopen(partition,"r");
    if (raw_f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, partition, strerror(errno));
        result = strdup("");
        goto done;
    }
    fclose(raw_f);
    raw_f = NULL;
    raw_f = fopen(partition,"wb");
    if (raw_f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, partition, strerror(errno));
        result = strdup("");
        goto done;
    }
    if (!!(fseek(raw_f, RADAR_IMAGE_OFFSET, SEEK_SET))) {
        fprintf(stderr, "[WriteRadarIamgeFn]fseek failed, %s\n", strerror(errno));
        result = strdup("");
        goto done;
    }
    fprintf(stderr, "[WriteRadarIamgeFn]fseek to write position: %d\n", RADAR_IMAGE_OFFSET);
    success = true;
    char* buffer = malloc(BUFSIZ);
    if (buffer == NULL) {
        success = false;
        ErrorAbort(state, "failed to alloc %d bytes", BUFSIZ);
        goto done_malloc;
    }
    int read;
    int wrote;
    int read_img_count = 0;
    int write_img_count = 0;
    while (success && (read = fread(buffer, 1, BUFSIZ, f)) > 0) {
        read_img_count += read;
        wrote = fwrite(buffer, 1, read, raw_f);
        write_img_count += wrote;
        success = success && (wrote == read);
        if (!success) {
            fprintf(stderr, "[WriteUbootFn]write to %s: %s\n",
                    partition, strerror(errno));
        }
    }
    fflush(raw_f);
    fsync(fileno(raw_f));

    free(buffer);

done_malloc:
    fclose(f);
    f = NULL;
    fclose(raw_f);
    raw_f = NULL;

    printf("%s %s partition from %s\n",
           success ? "wrote" : "failed to write", partition, filename);

    result = success ? partition : strdup("");

done:
    if (result != partition) free(partition);
    free(filename);
    return StringValue(result);
}

// write_uboot(file, partition)
Value* WriteUbootFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;

    char* partition;
    char* filename;
    long write_position;
    bool is_emmc = false;
    if (ReadArgs(state, argv, 2, &filename, &partition) < 0) {
        return NULL;
    }

    if (strcmp(partition, "/dev/mmcblk0") == 0) is_emmc = true;

    if (strlen(partition) == 0) {
        ErrorAbort(state, "partition argument to %s can't be empty", name);
        goto done;
    }
    if (strlen(filename) == 0) {
        ErrorAbort(state, "file argument to %s can't be empty", name);
        goto done;
    }
    bool success;
    fprintf(stderr, "[WriteUbootFn] open %s\n", filename);
    FILE* f = NULL;
    f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, filename, strerror(errno));
        result = strdup("");
        goto done;
    }
    long read_position;
    read_position = 0;
    if (!!(fseek(f, read_position, SEEK_SET))) {
        fprintf(stderr, "[WriteUbootFn]fseek failed, %s\n", strerror(errno));
        result = strdup("");
        goto done;
    }
    fprintf(stderr, "[WriteUbootFn]fseek to read position: %d\n", (int)read_position);
    fprintf(stderr, "[WriteUbootFn] open %s\n", partition);
    FILE* raw_f = NULL;
    raw_f = fopen(partition,"r");
    if (raw_f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, partition, strerror(errno));
        result = strdup("");
        goto done;
    }
    fclose(raw_f);
    raw_f = NULL;
    raw_f = fopen(partition,"wb");
    if (raw_f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, partition, strerror(errno));
        result = strdup("");
        goto done;
    }
    int fd;
    fd = open(partition, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, partition, strerror(errno));
        result = strdup("");
        goto done;
    }
    ioctl(fd, BLKSSZGET, &write_position);
    fprintf(stderr, "[WriteUbootFn]store uboot at offset %ld of %s\n",
        write_position, strerror(errno));
    close(fd);
    if (!!(fseek(raw_f, write_position, SEEK_SET))) {
        fprintf(stderr, "[WriteUbootFn]fseek failed, %s\n", strerror(errno));
        result = strdup("");
        goto done;
    }
    fprintf(stderr, "[WriteUbootFn]fseek to write position: %d\n", (int)write_position);
    success = true;
    char* buffer = malloc(BUFSIZ);
    if (buffer == NULL) {
        success = false;
        ErrorAbort(state, "failed to alloc %d bytes", BUFSIZ);
        goto done_malloc;
    }
    int read;
    int wrote;
    int read_img_count = 0;
    int write_img_count = 0;
    while (success && (read = fread(buffer, 1, BUFSIZ, f)) > 0) {
        read_img_count += read;
        wrote = fwrite(buffer, 1, read, raw_f);
        write_img_count += wrote;
        success = success && (wrote == read);
        if (!success) {
            fprintf(stderr, "[WriteUbootFn]write to %s: %s\n",
                    partition, strerror(errno));
        }
    }
    fflush(raw_f);
    fsync(fileno(raw_f));

    /* Write the backup uboot from 1MB + 1 sector for EMMC
     * NANDDISK do the backup internal
     * */
    if (is_emmc) {
	write_position += 1024 * 1024;
        if (!!(fseek(raw_f, write_position, SEEK_SET))) {
            fprintf(stderr, "[WriteUbootFn]fseek failed, %s\n", strerror(errno));
            result = strdup("");
            goto done;
        }
        fprintf(stderr, "[WriteUbootFn]fseek to write position: %d\n", (int)write_position);
        success = true;
	memset(buffer, 0, BUFSIZ);
        read = 0;
	wrote = 0;
        read_img_count = 0;
        write_img_count = 0;
        while (success && (read = fread(buffer, 1, BUFSIZ, f)) > 0) {
            read_img_count += read;
            wrote = fwrite(buffer, 1, read, raw_f);
            write_img_count += wrote;
            success = success && (wrote == read);
            if (!success) {
                fprintf(stderr, "[WriteUbootFn]write to %s: %s\n",
                        partition, strerror(errno));
            }
        }
        fflush(raw_f);
        fsync(fileno(raw_f));
    }

    // write commit flag
    if (!!(fseek(raw_f, COMMIT_FLAG_ADDR, SEEK_SET))) {
        fprintf(stderr, "[WriteUbootFn]fseek failed, %s\n", strerror(errno));
        result = strdup("");
        goto done;
    }
    success = success && (1 == fwrite(buffer, 1, 1, raw_f));
    if (!success) {
        fprintf(stderr, "[WriteUbootFn]write to %s: %s\n",
                partition, strerror(errno));
    }

    free(buffer);

    /* Write uboot update flag(0x3c) at the 2.5MB-4096+1 byte.
     * Uboot will check it to know if uboot has been updated.
     */
    int flag_offset = 0x0027f000 + 1;
    int flag = 0x3c;
    if (!(fseek(raw_f, flag_offset, SEEK_SET))) {
	fprintf(stderr, "[WriteUbootFn]set Uboot update flag %#x at %#x\n",flag, flag_offset);
	putc(flag, raw_f);
    } else {
        fprintf(stderr, "[WriteUbootFn]fseek Uboot update flag offset failed, %s\n", strerror(errno));
    }
    fflush(raw_f);
    fsync(fileno(raw_f));

done_malloc:
    fclose(f);
    f = NULL;
    fclose(raw_f);
    raw_f = NULL;

    printf("%s %s partition from %s\n",
           success ? "wrote" : "failed to write", partition, filename);

    result = success ? partition : strdup("");

done:
    if (result != partition) free(partition);
    free(filename);
    return StringValue(result);
}

// write_image_by_offset(file, partition, offset)
Value* WriteImageByOffsetFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;

    char* partition;
    char* filename;
    char* offset_string;
    size_t offset;
    if (ReadArgs(state, argv, 3, &filename, &partition, &offset_string) < 0) {
        return NULL;
    }

    if (strlen(partition) == 0) {
        ErrorAbort(state, "partition argument to %s can't be empty", name);
        goto done;
    }
    if (strlen(filename) == 0) {
        ErrorAbort(state, "file argument to %s can't be empty", name);
        goto done;
    }
    if (strlen(offset_string) == 0) {
        ErrorAbort(state, "offset argument to %s can't be empty", name);
        goto done;
    }
    offset = strtol(offset_string, NULL, 10);

    bool success;
    fprintf(stderr, "[WriteImageByOffset] open %s\n", filename);
    FILE* f = NULL;
    f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, filename, strerror(errno));
        result = strdup("");
        goto done;
    }
    FILE* raw_f = NULL;
    raw_f = fopen(partition, "r");
    if (raw_f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, partition, strerror(errno));
        result = strdup("");
        goto done;
    }
    fclose(raw_f);
    raw_f = NULL;
    raw_f = fopen(partition,"wb");
    if (raw_f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, partition, strerror(errno));
        result = strdup("");
        goto done;
    }
    if (!!(fseek(raw_f, offset, SEEK_SET))) {
        fprintf(stderr, "[WriteImageByOffset]fseek failed, %s\n", strerror(errno));
        result = strdup("");
        goto done;
    }
    fprintf(stderr, "[WriteImageByOffset]fseek to write position: %u\n", offset);
    success = true;
    char* buffer = malloc(BUFSIZ);
    if (buffer == NULL) {
        success = false;
        ErrorAbort(state, "failed to alloc %d bytes", BUFSIZ);
        goto done_malloc;
    }
    int read;
    int read_img_count = 0;
    int write_img_count = 0;
    while (success && (read = fread(buffer, 1, BUFSIZ, f)) > 0) {
        read_img_count += read;
        int wrote = fwrite(buffer, read, 1, raw_f);
        write_img_count += wrote;
        success = success && (wrote == read);
        if (!success) {
            fprintf(stderr, "[WriteImageByOffset]write to %s: %s\n",
                    partition, strerror(errno));
        }
    }
    free(buffer);
done_malloc:
    fclose(f);
    f = NULL;
    fclose(raw_f);
    raw_f = NULL;

    printf("%s %s partition from %s\n",
           success ? "wrote" : "failed to write", partition, filename);

    result = success ? partition : strdup("");

done:
    if (result != partition) free(partition);
    free(filename);
    return StringValue(result);
}

// write_raw_image(filename_or_blob, partition)
Value* WriteRawImageFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;

    Value* partition_value;
    Value* contents;
    if (ReadValueArgs(state, argv, 2, &contents, &partition_value) < 0) {
        return NULL;
    }

    if (partition_value->type != VAL_STRING) {
        ErrorAbort(state, "partition argument to %s must be string", name);
        goto done;
    }
    char* partition = partition_value->data;
    if (strlen(partition) == 0) {
        ErrorAbort(state, "partition argument to %s can't be empty", name);
        goto done;
    }
    if (contents->type == VAL_STRING && strlen((char*) contents->data) == 0) {
        ErrorAbort(state, "file argument to %s can't be empty", name);
        goto done;
    }

    mtd_scan_partitions();
    const MtdPartition* mtd = mtd_find_partition_by_name(partition);
    if (mtd == NULL) {
        fprintf(stderr, "%s: no mtd partition named \"%s\"\n", name, partition);
        result = strdup("");
        goto done;
    }

    MtdWriteContext* ctx = mtd_write_partition(mtd);
    if (ctx == NULL) {
        fprintf(stderr, "%s: can't write mtd partition \"%s\"\n",
                name, partition);
        result = strdup("");
        goto done;
    }

    bool success;

    if (contents->type == VAL_STRING) {
        // we're given a filename as the contents
        char* filename = contents->data;
        FILE* f = fopen(filename, "rb");
        if (f == NULL) {
            fprintf(stderr, "%s: can't open %s: %s\n",
                    name, filename, strerror(errno));
            result = strdup("");
            goto done;
        }

        success = true;
        char* buffer = malloc(BUFSIZ);
        int read;
        while (success && (read = fread(buffer, 1, BUFSIZ, f)) > 0) {
            int wrote = mtd_write_data(ctx, buffer, read);
            success = success && (wrote == read);
        }
        free(buffer);
        fclose(f);
    } else {
        // we're given a blob as the contents
        ssize_t wrote = mtd_write_data(ctx, contents->data, contents->size);
        success = (wrote == contents->size);
    }
    if (!success) {
        fprintf(stderr, "mtd_write_data to %s failed: %s\n",
                partition, strerror(errno));
    }

    if (mtd_erase_blocks(ctx, -1) == -1) {
        fprintf(stderr, "%s: error erasing blocks of %s\n", name, partition);
    }
    if (mtd_write_close(ctx) != 0) {
        fprintf(stderr, "%s: error closing write of %s\n", name, partition);
    }

    printf("%s %s partition\n",
           success ? "wrote" : "failed to write", partition);

    result = success ? partition : strdup("");

done:
    if (result != partition) FreeValue(partition_value);
    FreeValue(contents);
    return StringValue(result);
}

// apply_patch_space(bytes)
Value* ApplyPatchSpaceFn(const char* name, State* state,
                         int argc, Expr* argv[]) {
    char* bytes_str;
    if (ReadArgs(state, argv, 1, &bytes_str) < 0) {
        return NULL;
    }

    char* endptr;
    size_t bytes = strtol(bytes_str, &endptr, 10);
    if (bytes == 0 && endptr == bytes_str) {
        ErrorAbort(state, "%s(): can't parse \"%s\" as byte count\n\n",
                   name, bytes_str);
        free(bytes_str);
        return NULL;
    }

    return StringValue(strdup(CacheSizeCheck(bytes) ? "" : "t"));
}


// apply_patch(srcfile, tgtfile, tgtsha1, tgtsize, sha1_1, patch_1, ...)
Value* ApplyPatchFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc < 6 || (argc % 2) == 1) {
        return ErrorAbort(state, "%s(): expected at least 6 args and an "
                                 "even number, got %d",
                          name, argc);
    }

    char* source_filename;
    char* target_filename;
    char* target_sha1;
    char* target_size_str;
    if (ReadArgs(state, argv, 4, &source_filename, &target_filename,
                 &target_sha1, &target_size_str) < 0) {
        return NULL;
    }

    char* endptr;
    size_t target_size = strtol(target_size_str, &endptr, 10);
    if (target_size == 0 && endptr == target_size_str) {
        ErrorAbort(state, "%s(): can't parse \"%s\" as byte count",
                   name, target_size_str);
        free(source_filename);
        free(target_filename);
        free(target_sha1);
        free(target_size_str);
        return NULL;
    }

    int patchcount = (argc-4) / 2;
    Value** patches = ReadValueVarArgs(state, argc-4, argv+4);

    int i;
    for (i = 0; i < patchcount; ++i) {
        if (patches[i*2]->type != VAL_STRING) {
            ErrorAbort(state, "%s(): sha-1 #%d is not string", name, i);
            break;
        }
        if (patches[i*2+1]->type != VAL_BLOB) {
            ErrorAbort(state, "%s(): patch #%d is not blob", name, i);
            break;
        }
    }
    if (i != patchcount) {
        for (i = 0; i < patchcount*2; ++i) {
            FreeValue(patches[i]);
        }
        free(patches);
        return NULL;
    }

    char** patch_sha_str = malloc(patchcount * sizeof(char*));
    for (i = 0; i < patchcount; ++i) {
        patch_sha_str[i] = patches[i*2]->data;
        patches[i*2]->data = NULL;
        FreeValue(patches[i*2]);
        patches[i] = patches[i*2+1];
    }

    int result = applypatch(source_filename, target_filename,
                            target_sha1, target_size,
                            patchcount, patch_sha_str, patches);

    for (i = 0; i < patchcount; ++i) {
        FreeValue(patches[i]);
    }
    free(patch_sha_str);
    free(patches);

    return StringValue(strdup(result == 0 ? "t" : ""));
}

// apply_patch_check(file, [sha1_1, ...])
Value* ApplyPatchCheckFn(const char* name, State* state,
                         int argc, Expr* argv[]) {
    if (argc < 1) {
        return ErrorAbort(state, "%s(): expected at least 1 arg, got %d",
                          name, argc);
    }

    char* filename;
    if (ReadArgs(state, argv, 1, &filename) < 0) {
        return NULL;
    }

    int patchcount = argc-1;
    char** sha1s = ReadVarArgs(state, argc-1, argv+1);

    int result = applypatch_check(filename, patchcount, sha1s);

    int i;
    for (i = 0; i < patchcount; ++i) {
        free(sha1s[i]);
    }
    free(sha1s);

    return StringValue(strdup(result == 0 ? "t" : ""));
}

Value* UIPrintFn(const char* name, State* state, int argc, Expr* argv[]) {
    char** args = ReadVarArgs(state, argc, argv);
    if (args == NULL) {
        return NULL;
    }

    int size = 0;
    int i;
    for (i = 0; i < argc; ++i) {
        size += strlen(args[i]);
    }
    char* buffer = malloc(size+1);
    size = 0;
    for (i = 0; i < argc; ++i) {
        strcpy(buffer+size, args[i]);
        size += strlen(args[i]);
        free(args[i]);
    }
    free(args);
    buffer[size] = '\0';

    char* line = strtok(buffer, "\n");
    while (line) {
        fprintf(((UpdaterInfo*)(state->cookie))->cmd_pipe,
                "ui_print %s\n", line);
        line = strtok(NULL, "\n");
    }
    fprintf(((UpdaterInfo*)(state->cookie))->cmd_pipe, "ui_print\n");

    return StringValue(buffer);
}

Value* WipeCacheFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc != 0) {
        return ErrorAbort(state, "%s() expects no args, got %d", name, argc);
    }
    fprintf(((UpdaterInfo*)(state->cookie))->cmd_pipe, "wipe_cache\n");
    return StringValue(strdup("t"));
}

Value* RunProgramFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc < 1) {
        return ErrorAbort(state, "%s() expects at least 1 arg", name);
    }
    char** args = ReadVarArgs(state, argc, argv);
    if (args == NULL) {
        return NULL;
    }

    char** args2 = malloc(sizeof(char*) * (argc+1));
    memcpy(args2, args, sizeof(char*) * argc);
    args2[argc] = NULL;

    fprintf(stderr, "about to run program [%s] with %d args\n", args2[0], argc);

    pid_t child = fork();
    if (child == 0) {
        execv(args2[0], args2);
        fprintf(stderr, "run_program: execv failed: %s\n", strerror(errno));
        _exit(1);
    }
    int status;
    waitpid(child, &status, 0);
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != 0) {
            fprintf(stderr, "run_program: child exited with status %d\n",
                    WEXITSTATUS(status));
        }
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "run_program: child terminated by signal %d\n",
                WTERMSIG(status));
    }

    int i;
    for (i = 0; i < argc; ++i) {
        free(args[i]);
    }
    free(args);
    free(args2);

    char buffer[20];
    sprintf(buffer, "%d", status);

    return StringValue(strdup(buffer));
}

// Take a sha-1 digest and return it as a newly-allocated hex string.
static char* PrintSha1(uint8_t* digest) {
    char* buffer = malloc(SHA_DIGEST_SIZE*2 + 1);
    int i;
    const char* alphabet = "0123456789abcdef";
    for (i = 0; i < SHA_DIGEST_SIZE; ++i) {
        buffer[i*2] = alphabet[(digest[i] >> 4) & 0xf];
        buffer[i*2+1] = alphabet[digest[i] & 0xf];
    }
    buffer[i*2] = '\0';
    return buffer;
}

// sha1_check(data)
//    to return the sha1 of the data (given in the format returned by
//    read_file).
//
// sha1_check(data, sha1_hex, [sha1_hex, ...])
//    returns the sha1 of the file if it matches any of the hex
//    strings passed, or "" if it does not equal any of them.
//
Value* Sha1CheckFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc < 1) {
        return ErrorAbort(state, "%s() expects at least 1 arg", name);
    }

    Value** args = ReadValueVarArgs(state, argc, argv);
    if (args == NULL) {
        return NULL;
    }

    if (args[0]->size < 0) {
        fprintf(stderr, "%s(): no file contents received", name);
        return StringValue(strdup(""));
    }
    uint8_t digest[SHA_DIGEST_SIZE];
    SHA(args[0]->data, args[0]->size, digest);
    FreeValue(args[0]);

    if (argc == 1) {
        return StringValue(PrintSha1(digest));
    }

    int i;
    uint8_t* arg_digest = malloc(SHA_DIGEST_SIZE);
    for (i = 1; i < argc; ++i) {
        if (args[i]->type != VAL_STRING) {
            fprintf(stderr, "%s(): arg %d is not a string; skipping",
                    name, i);
        } else if (ParseSha1(args[i]->data, arg_digest) != 0) {
            // Warn about bad args and skip them.
            fprintf(stderr, "%s(): error parsing \"%s\" as sha-1; skipping",
                    name, args[i]->data);
        } else if (memcmp(digest, arg_digest, SHA_DIGEST_SIZE) == 0) {
            break;
        }
        FreeValue(args[i]);
    }
    if (i >= argc) {
        // Didn't match any of the hex strings; return false.
        return StringValue(strdup(""));
    }
    // Found a match; free all the remaining arguments and return the
    // matched one.
    int j;
    for (j = i+1; j < argc; ++j) {
        FreeValue(args[j]);
    }
    return args[i];
}

// Read a local file and return its contents (the Value* returned
// is actually a FileContents*).
Value* ReadFileFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 1 arg, got %d", name, argc);
    }
    char* filename;
    if (ReadArgs(state, argv, 1, &filename) < 0) return NULL;

    Value* v = malloc(sizeof(Value));
    v->type = VAL_BLOB;

    FileContents fc;
    if (LoadFileContents(filename, &fc, RETOUCH_DONT_MASK) != 0) {
        ErrorAbort(state, "%s() loading \"%s\" failed: %s",
                   name, filename, strerror(errno));
        free(filename);
        free(v);
        free(fc.data);
        return NULL;
    }

    v->size = fc.size;
    v->data = (char*)fc.data;

    free(filename);
    return v;
}

//is_location_valid("location")
Value* IsLocationValidFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 1 args, got %d", name, argc);
    }
    char* location;
    if (ReadArgs(state, argv, 1, &location) < 0) {
        return NULL;
    }

    if (strlen(location) == 0) {
        ErrorAbort(state, "location argument to %s() can't be empty", name);
        goto done;
    }
    FILE* f = fopen(location,"r");
    if (f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, location, strerror(errno));
        result = strdup("");
        goto done;
    }
    fclose(f);
    result = location;
done:
    if (result != location) free(location);
    return StringValue(result);
}

#define SIZE_1K                 1024
#define SIZE_1M                 (1024*1024)
#define OEMDATA_OFFSET          (5*SIZE_1M)

#define RAW_ZONE                "/dev/mmcblk0"
#define SVP_PKG_NAME            "svp.bin"
#define APP_PKG_NAME            "app-pkg.bin"
#define FACTORY_PKG             "factory_update"

#define UPGRADE_OS		(1U << 0)
#define UPGRADE_SVP		(1U << 1)
#define UPGRADE_APP		(1U << 2)
#define UPGRADE_LOGO		(1U << 3)
#define UPGRADE_RADIO		(1U << 4)
#define UPGRADE_DSP		(1U << 5)
#define UPGRADE_GPS		(1U << 6)
#define UPDATE_ALL              (0xffffffff)

bool check_filename_with_packages(unsigned int packages, char * str)
{
        if (((strcmp(str, FACTORY_PKG) == 0) && (packages == UPDATE_ALL)) ||
           ((strcmp(str, APP_PKG_NAME) == 0) && (packages & UPGRADE_APP == UPGRADE_APP)) ||
           ((strcmp(str, SVP_PKG_NAME) == 0) && (packages & UPGRADE_SVP == UPGRADE_SVP)))
                return 1;

        return 0;
} 

//is_valid_update("svp.bin")
Value* IsValidUpdateFn (const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 1 args, got %d", name, argc);
    }
    char* location;
    if (ReadArgs(state, argv, 1, &location) < 0) {
        return NULL;
    }

    if (strlen(location) == 0) {
        ErrorAbort(state, "location argument to %s() can't be empty", name);
        goto done;
    }
    
    FILE* raw_f = NULL;
    raw_f = fopen(RAW_ZONE,"r");
    if (raw_f == NULL) {
        fprintf(stderr, "%s: can't open %s: %s\n",
                name, RAW_ZONE, strerror(errno));
        result = strdup("");
        goto done_malloc;
    }

    if (!!(fseek(raw_f, OEMDATA_OFFSET, SEEK_SET))) {
        fprintf(stderr, "[WriteImageByOffset]fseek failed, %s\n", strerror(errno));
        result = strdup("");
        goto done;
    }
    fprintf(stderr, "[WriteImageByOffset]fseek to write position: %u\n", OEMDATA_OFFSET);

    int data = 0;
    char* buffer = malloc(BUFSIZ);
    if (buffer == NULL) {
        ErrorAbort(state, "failed to alloc %d bytes", BUFSIZ);
        goto done_malloc;
    }
    int read;
    if((read = fread(buffer, sizeof(int), BUFSIZ, raw_f)) > 0) {
        data = ((int*)buffer)[0];
        fprintf(stderr, "OS:MSG: read oemdata:  %u\n", data);
    }

    fprintf(stderr, "OS:MSG: free++!\n");
    free(buffer);
    fprintf(stderr, "OS:MSG: free--!\n");

    
    if(check_filename_with_packages(data, location))
    {
        result = location;
        fprintf(stderr, "OS:MSG: check_filename_with_packages:%s\n", result);
    }
    else
    {
        result = strdup("");
    }
    
done_malloc:
    fclose(raw_f);
    raw_f = NULL;
 
done:
    if (result != location) free(location);
    fprintf(stderr, "%s: result=%s\n", name, result);
    return StringValue(result);
}

//get_image_index(cfg_file_path, index_type))
Value* GetImageIndexFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* cfg_file_path = NULL;
    char* index_type = NULL;
    char* result = NULL;
    char index;
    char buffer[BUFSIZ];
    FILE *fp;

    if (argc != 2) {
        return ErrorAbort(state, "%s() expects 2 args, got %d", name, argc);
    }
    if (ReadArgs(state, argv, 2, &cfg_file_path, &index_type) < 0)
        return NULL;
    if (strlen(cfg_file_path) == 0) {
        ErrorAbort(state, "cfg_file_path argument to %s() can't be empty", name);
        goto done;
    }
    if (strlen(index_type) == 0) {
        ErrorAbort(state, "index_type argument to %s() can't be empty", name);
        goto done;
    }

    FILE* f = fopen(cfg_file_path, "rw");
    if (f == NULL) {
        fprintf(stderr, "%s: can't open %s for read&write: %s\n",
                name, cfg_file_path, strerror(errno));
        goto done;
    }

    fread(buffer, 1, BUFSIZ, f);
    char *p, *q;
    if ((p = strstr(buffer, index_type)) != NULL) {
        while(*p++ != '=');
        q=p;
        while((*p != ';') && (*p != '\0')) p++;
        *p='\0';
        result = strdup(q);
        goto done;
    }

done:
    fclose(f);
    free(cfg_file_path);
    free(index_type);
    if (result == NULL) result = strdup("");
    fprintf(stderr, "%s: result=%s\n", name, result);
    return StringValue(result);
}

//set_image_index(cfg_file_path, image_type, index_value))
Value* SetImageIndexFn(const char* name, State* state, int argc, Expr* argv[]) {
    char* result = NULL;
    char* cfg_file_path = NULL;
    char* index_type = NULL;
    char* index_value = NULL;
    char index;
    char* buffer = NULL;
    char* p;
    FILE* fp;
    int file_len;
    int read_bytes;

    if (argc != 3) {
        return ErrorAbort(state, "%s() expects 3 args, got %d", name, argc);
    }
    if (ReadArgs(state, argv, 3, &cfg_file_path, &index_type, &index_value) < 0)
        return NULL;
    if (strlen(cfg_file_path) == 0) {
        ErrorAbort(state, "cfg_file_path argument to %s() can't be empty", name);
        goto done;
    }
    if (strlen(index_type) == 0) {
        ErrorAbort(state, "index_type argument to %s() can't be empty", name);
        goto done;
    }
    if (strlen(index_value) == 0) {
        ErrorAbort(state, "index_type argument to %s() can't be empty", name);
        goto done;
    }

    fp = fopen(cfg_file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: can't open %s for read: %s\n",
                name, cfg_file_path, strerror(errno));
        goto done;
    }

    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = malloc(file_len);
    read_bytes = fread(buffer, 1, file_len, fp);
    fprintf(stderr, "%s: file len=%d, read_bytes =%d, buffer=%s, index_type=%s, index_value=%s\n", name, file_len, read_bytes, buffer, index_type, index_value);
    fclose(fp);
    if ((p = strstr(buffer, index_type)) != NULL) {
        while(*p++ != '=');
        int offset = p - buffer;
        *p = *index_value;
        fp = fopen(cfg_file_path, "w+");
        fseek(fp, 0, SEEK_SET);
        fwrite(buffer, 1, read_bytes, fp);
        result = cfg_file_path;
        goto done;
    }
    result = strdup("");

done:
    fclose(fp);
    free(index_type);
    free(index_value);
    if (result != cfg_file_path) free(cfg_file_path);
    fprintf(stderr, "%s: result=%s\n", name, result);
    return StringValue(result);
}

void RegisterInstallFunctions() {
    RegisterFunction("mount", MountFn);
    RegisterFunction("is_mounted", IsMountedFn);
    RegisterFunction("unmount", UnmountFn);
    RegisterFunction("format", FormatFn);
    RegisterFunction("format_ext4", FormatExt4Fn);
    RegisterFunction("show_progress", ShowProgressFn);
    RegisterFunction("set_progress", SetProgressFn);
    RegisterFunction("delete", DeleteFn);
    RegisterFunction("delete_recursive", DeleteFn);
    RegisterFunction("package_extract_dir", PackageExtractDirFn);
    RegisterFunction("package_extract_file", PackageExtractFileFn);
    RegisterFunction("retouch_binaries", RetouchBinariesFn);
    RegisterFunction("undo_retouch_binaries", UndoRetouchBinariesFn);
    RegisterFunction("symlink", SymlinkFn);
    RegisterFunction("set_perm", SetPermFn);
    RegisterFunction("set_perm_recursive", SetPermFn);

    RegisterFunction("getprop", GetPropFn);
    RegisterFunction("file_getprop", FileGetPropFn);
    RegisterFunction("write_raw_image", WriteRawImageFn);
    RegisterFunction("write_freertos", WriteFreertosFn);
    RegisterFunction("write_uboot", WriteUbootFn);
    RegisterFunction("write_image_by_offset", WriteImageByOffsetFn);

    RegisterFunction("apply_patch", ApplyPatchFn);
    RegisterFunction("apply_patch_check", ApplyPatchCheckFn);
    RegisterFunction("apply_patch_space", ApplyPatchSpaceFn);

    RegisterFunction("read_file", ReadFileFn);
    RegisterFunction("sha1_check", Sha1CheckFn);

    RegisterFunction("wipe_cache", WipeCacheFn);

    RegisterFunction("ui_print", UIPrintFn);

    RegisterFunction("run_program", RunProgramFn);
    RegisterFunction("is_location_valid", IsLocationValidFn);
    RegisterFunction("get_image_index", GetImageIndexFn);
    RegisterFunction("set_image_index", SetImageIndexFn);

    RegisterFunction("is_valid_update", IsValidUpdateFn);
    RegisterFunction("extract_dir", ExtractDirFn);
    RegisterFunction("write_radar_image", WriteRadarIamgeFn);
}

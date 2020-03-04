/*
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "TP_CALIBRATION"

#define LOG_NDEBUG 0
#include <cutils/log.h>
#include <nativehelper/jni.h>
#include <nativehelper/JNIHelp.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cutils/properties.h>
#include <cutils/native_handle.h>
namespace android {

char* jstringTostring(JNIEnv* env, jstring jstr)
{
      char* rtn = NULL;
      jclass clsstring = env->FindClass("java/lang/String");
      jstring strencode = env->NewStringUTF("utf-8");
      jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
      jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
      jsize alen = env->GetArrayLength(barr);
      jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
      if (alen > 0)
      {
                rtn = (char*)malloc(alen + 1);
                memcpy(rtn, ba, alen);
                rtn[alen] = 0;
      }
      env->ReleaseByteArrayElements(barr, ba, 0);
      return rtn;
}

static int fd;

jint Java_com_android_calibration_NativeCalibration_open(JNIEnv *env, jobject clazz, jstring pathname)
{
  const char *name = jstringTostring(env, pathname);
  SLOGD("NativeCalibration OPEN:%s \n", name);
  fd = open(name, O_RDWR);
  SLOGD("fd = %d \n", fd);
  if (fd < 0) {
	SLOGE("open tp device failed!");
	return fd;
  }
  return 0;
}

jint Java_com_android_calibration_NativeCalibration_ioctl(JNIEnv *env, jobject clazz, jint cmd)
{
   int ret;
   SLOGD("TP ioctl function");
   ret = ioctl(fd, cmd, 0);
   if (ret < 0) {
      SLOGE("Tp device ioctl operation failed!");
      return ret;
   }
   return 0;
}

 jint  Java_com_android_calibration_NativeCalibration_write(JNIEnv *env, jobject clazz, jstring buf)
{
    int ret;
    SLOGD("NativeCalibration WRITE\n");
    const char *wr_buf = jstringTostring(env, buf);
    ret = write(fd, wr_buf, 2);
    if (ret != 2) {
      SLOGE("TP device write operation failed!");
      return ret;
    }
   return 0;
}
jint Java_com_android_calibration_NativeCalibration_close(JNIEnv *env, jobject clazz)
{
    if (fd)
	close(fd);
    return 0;
}

static JNINativeMethod gMethods[] = {
    {"open", "(Ljava/lang/String;)I", (void*) Java_com_android_calibration_NativeCalibration_open},
    {"close", "()I", (void*)Java_com_android_calibration_NativeCalibration_close },
    {"write", "(Ljava/lang/String;)I", (void*) Java_com_android_calibration_NativeCalibration_write},
    {"ioctl", "(I)I", (void*)Java_com_android_calibration_NativeCalibration_ioctl},
};
}; // namespace android
using namespace android;
static int registerNatives(JNIEnv* env)
{
  SLOGI("RegisterNatives");
  if (!jniRegisterNativeMethods(env, "com/android/calibration/NativeCalibration", gMethods, NELEM(gMethods))) {
      SLOGI("Register Native Methods is success");
      return JNI_TRUE;
  }
  return JNI_FALSE;
}
typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;
    
    SLOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;
    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto bail;
    }
    result = JNI_VERSION_1_4;
    
bail:
    return result;
}

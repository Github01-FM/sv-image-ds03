package com.android.calibration;
public class NativeCalibration
{
	private static final String TAG = "TpCalibrate";
	
	public static final int CALIBRATION_FLAG = 1;
	public static final int NORMAL_MODE = 8;
	public static final int DEBUG_MODE = 3;
	public static final int INTERNAL_MODE = 4;
	public static final int RASTER_MODE	= 5;
	public static final int VERSION_FLAG = 6;
	public static final int BOOTLOADER_MODE = 7;
	public static final int BUTTON_RAW = 2;
	public static final int REEPROM = 12;
	public static final int WEEPROM = 13;
	public static final int ENABLE_IRQ = 10;
	public static final int DISABLE_IRQ = 11;
	static {
		System.loadLibrary("calibrate_jni");
	}

	public static native int open(String fname);
	public static native int close();
	public static native int ioctl(int cmd);
	public static native int write(String str);
};

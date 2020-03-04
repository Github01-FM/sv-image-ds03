//package com.telink.tscalibration;
package com.telink.tscalibrationv2;

import java.io.File;

import android.graphics.Point;

public interface Operation {
	public abstract  String getStringFormFile(File file);
	public abstract  boolean doCheckCalibration();
	public abstract  boolean writeStringToFile(File file, String str);
	public abstract  Point getPointAdc();
	public abstract  void setCalibrateAD(Point[] calibrateAD);
	public abstract  boolean getTLCalResult();
	public abstract  Point getResFromDrv();	
	public abstract void delFile(String filePathAndName);
	public abstract void newFile(String filePathAndName, String fileContent); 
	public abstract  void LoadCoeFromFile();
}

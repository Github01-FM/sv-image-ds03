//package com.telink.tscalibration;
package com.telink.tscalibrationv2;

import java.io.File;
import java.lang.System;
import java.lang.Integer;

import android.app.Service;
import android.os.IBinder;
import android.util.Log;
import android.content.Intent;
import android.graphics.Point;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.ComponentName;  

import android.provider.Settings;
import android.os.SystemProperties;

public class AutoCalibration extends Service {
	
	private static final String TAG = "cali-service";
	private AppToDriver mATD= new AppToDriver();
	private File saveToPreference = new File("/data/data/com.telink.tscalibrationv2/shared_prefs/TouchCheck.xml");
	
	public boolean LoadCalibrateFormPrefFile()
	{   
	    Log.d(TAG, "LoadCalibrateFormPrefFile....");
	    Point[] calibrateAD = new Point[5];
	    if(!saveToPreference.exists() || saveToPreference==null ) {
	    	Log.v(TAG,"file lose:"+saveToPreference);
	    	return false;
	    }
	    SharedPreferences prefCalibrate = getSharedPreferences(AppToDriver.TOUCH_CHECK_VALUE, 0); 
	    	
	    for (int i = 0; i <mATD.TOUCH_CHECK_END; i++)
	    {   
	    	calibrateAD[i] = new Point(-1, -1);
            calibrateAD[i].x = prefCalibrate.getInt(mATD.strTouch[i*2+0], -1);
	    	calibrateAD[i].y = prefCalibrate.getInt(mATD.strTouch[i*2+1], -1);
	        //Log.d(TAG, "SN="+i+"  x="+calibrateAD[i].x+"  y="+calibrateAD[i].y);

	        if((calibrateAD[i].x <= 0) || (calibrateAD[i].y <= 0)) 
	            return false;

	    }   
	    //Write the Calibration parameters to TP driver.
	    mATD.setCalibrateAD(calibrateAD);
	    if (mATD.doCheckCalibration() == true)
	        return true;

	    return false;
	}   

	@Override
	public IBinder onBind(Intent intent) {
        return null;
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        boolean ret = LoadCalibrateFormPrefFile();
        int prop = Integer.parseInt(SystemProperties.get("ro.secure.tscalibration","0"));
      	int firstBoot = Settings.Secure.getInt(getContentResolver(),Settings.Secure.DEVICE_PROVISIONED,0);

		//andy0717, try to delete the following;

		//andy0724, test whether downloading coe is OK;
		mATD.LoadCoeFromFile();

		/*/     //andy0729, remove the following code;
        if (ret == false && prop == 1 && firstBoot != 0) {
			PackageManager pm = getPackageManager();  
			ComponentName name = new ComponentName(this, TSCalibration.class);  
			
			if(pm.getComponentEnabledSetting(name) == PackageManager.COMPONENT_ENABLED_STATE_DISABLED) {
			      pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_ENABLED,PackageManager.DONT_KILL_APP);
			
			    Intent tscheck=new Intent(this,TSCalibration.class);
			    tscheck.putExtra("enabletouchcheck","enabletouchcheck");
			    tscheck.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
			                        | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
			    startActivity(tscheck);
            }
        }
		/*/
    }
    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}

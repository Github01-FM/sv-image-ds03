//package com.telink.tscalibration;
package com.telink.tscalibrationv2;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;

import android.app.Activity;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.content.ComponentName;  
import android.content.SharedPreferences;
import android.content.pm.PackageManager;  
import android.content.Intent;
import android.graphics.Point;

/*/

public class TouchScreenMonitor extends Activity {

	private final String TAG = "TSMonitor";
	
	@Override  
	public void onCreate(Bundle service) {  
	    super.onCreate(service);
		
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
	
}
/*/


//package com.telink.tscalibration;
package com.telink.tscalibrationv2;

import android.content.BroadcastReceiver; 
import android.content.Context;
import android.content.Intent;

public class CalibrationReceiver extends BroadcastReceiver {

     static  final String ACTION="android.intent.action.BOOT_COMPLETED" ;
     @Override
     public   void  onReceive(Context context, Intent intent) {
         //if(false) {
        if  (intent.getAction().equals(ACTION)) {
        	 Intent i=new Intent();
        	 i.setClass(context, AutoCalibration.class);
        	 i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        	 context.startService(i);
			
        }
    }
}

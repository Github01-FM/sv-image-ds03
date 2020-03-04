//package com.telink.tscalibration;
package com.telink.tscalibrationv2;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.io.*; //andy0716;


import android.content.SharedPreferences;
import android.graphics.Point;
import android.os.Bundle;
import android.util.Log;
import android.app.Activity;

public class AppToDriver implements Operation{
	private final String TAG = "ATD";

	public int tst =0;
	
	public File fileAdValue = new File("/sys/class/touchpanel/touchadc"); 
	public File fileAdCalibrate = new File("/sys/class/touchpanel/touchcheck");
	public File caliStatus = new File("/sys/class/touchpanel/calistatus"); 


	//ts351x_cal interface is used by the cal flow, step by step calibration procedure;
	//ts351x_cal_set, tell drv that the present cal step id;
	//ts351x_cal_get, a interface can be used by App to poll the result;

	//ts351x_coe_set, set/download coe to ts351x;
	//ts351x_coe_get, read coe from ts351x(if forced, otherwise coe reported is the one cached by drv);
	
	public File fileTs351xCal = new File("/sys/class/telinktp/ts351xcal");	
	//public File fileTs351xCoe = new File("/sys/class/telinktp/ts351xcoe");

	//fileDrv351xCalReal is used for debug only: test communicate path between Ap and Drv;
	//public File fileDrv351xCalReal = new File("/sys/devices/platform/pxa2xx-i2c.0/i2c-0/0-002e/cal2");
	
	//caldd is dummy, used to debug the main flow only;
	//public File fileDrv351xCal = new File("/sys/devices/platform/pxa2xx-i2c.0/i2c-0/0-002e/caldd");
	public File fileDrv351xCal = new File("/sys/devices/platform/pxa2xx-i2c.0/i2c-0/0-002e/cal2dd");
	public File fileCoeSave = new File("/data/local/ts351xcoe");
	public String strTs351xCoeFile ="/data/local/ts351xcoe";
	public File filepath_cal2 = new File("/data/local/ts351x_calpath"); //to be used by the service;

	//public File ts351x_calif;
	
	public String[] strTouch = new String[]{"CENTER_X","CENTER_Y",
																 "TOP_LEFT_X","TOP_LEFT_Y",
																 "TOP_RIGHT_X","TOP_RIGHT_Y",
																 "BOTTOM_LEFT_X","BOTTOM_LEFT_Y",
																 "BOTTOM_RIGHT_X","BOTTOM_RIGHT_Y"};

	public final int TOUCH_TOP_LEFT     = 0;
	public final int TOUCH_TOP_RIGHT    = 1;
	public final int TOUCH_BOTTOM_LEFT  = 2;
	public final int TOUCH_BOTTOM_RIGHT = 3;
	public final int TOUCH_CENTER       = 4;
	public final int TOUCH_CHECK_END	 = 5;
	
	public final static String TOUCH_CHECK_VALUE = "TouchCheck";
	
	public AppToDriver () {
		
			if(!fileDrv351xCal.exists())// if(false) //if(!fileDrv351xCal.exists())
			{
				Log.d(TAG, "ts351x Java cal: try to load cal if via path file.\n");
				String path_ts351x_calif ="";
				if(!filepath_cal2.exists())
					Log.d(TAG, "Platform dependent File /data/local/ts351x_calpath not exist!\n");
				path_ts351x_calif =getStringFormFile(filepath_cal2);
				path_ts351x_calif += "/cal2";
				fileDrv351xCal = new File(path_ts351x_calif);
				//writeStringToFile(ts351x_calif, strCoeCmd);

				if(!fileDrv351xCal.exists())
				{
					Log.d(TAG, "Failed to load ts351x cal interface file!\n");
				}
			}
		
		
	}
	public String getStringFormFile(File file)
	{
		
        if (!file.exists()) 
		{
			Log.d(TAG, "File not exist");
            return null;
        }
        try {
            RandomAccessFile raf = new RandomAccessFile(file, "r");
			return raf.readLine();
        } catch (IOException e) {
            Log.w(TAG, "Exception opening file: " + file.getAbsolutePath(), e);
        }
        return null;
	}
	public boolean doCheckCalibration() {

		String status = getStringFormFile(caliStatus); //From driver

		if (status != null && status.length()>0)
		{
			Log.v("ATD","status"+status);
            if (status.equals("successful"))
				return true;
		}

		return false;
	}
	
	public boolean writeStringToFile(File file, String str)
	{
		//return false; //andy0730, test only;
		
        if (!file.exists()) 
		{
			Log.d(TAG, "File not exist");
            return false;
        }
        try {
            RandomAccessFile raf = new RandomAccessFile(file, "rw");
			char [] toChar = str.toCharArray();
			byte[] buffer = new byte[toChar.length];
			for(int i=0; i<buffer.length; i++)
			{
				buffer[i] = (byte)toChar[i];
//				Log.d(TAG, i+":"+buffer[i]);
			}
			raf.write(buffer);
			return true;
        } catch (IOException e) {
            Log.w(TAG, "Exception opening file: " + file.getAbsolutePath(), e);
        }
        return false;
	}

	public void delFile(String filePathAndName) { 
	   try { 
		 String filePath = filePathAndName; 
		 filePath = filePath.toString(); 
		 java.io.File myDelFile = new java.io.File(filePath); 
		 myDelFile.delete(); 
	
	   } 
	   catch (Exception e) { 
		 //System.out.println("err delete file,"); 
		 //e.printStackTrace();
		 Log.e("ATD", "err delete file:"+filePathAndName);
	
	   }
	
	 }
	
	public void newFile(String filePathAndName, String fileContent) { 
	
		try { 
		  String filePath = filePathAndName; 
		  filePath = filePath.toString(); 
		  File myFilePath = new File(filePath); 
		  if (!myFilePath.exists()) {		  
				myFilePath.createNewFile();	  	
		  } 
		  FileWriter resultFile = new FileWriter(myFilePath); 
		  PrintWriter myFile = new PrintWriter(resultFile); 
		  String strContent = fileContent; 
		  myFile.println(strContent); 
		  resultFile.close(); 
	
		} 
		catch (Exception e) { 
		  //System.out.println("write new file failed"); 
		  e.printStackTrace(); 
		  Log.e("ATD", "err write new file:"+filePathAndName+";"+ fileContent);
	
		}
	
	  }

	//getResFromDrv
	public Point getResFromDrv()
	{
		Point ptRes = new Point(0, 0);
		//String drvRsp = getStringFormFile(fileDrv351xCalReal); //From driver
		String drvCoeInt =""; //andy0716, test only,
		String drvCmd;
		String drvRsp;

		drvCmd = String.format("%d,%d,%d,%d,",109, 0, 16, 18);

		writeStringToFile(fileDrv351xCal, drvCmd);
		
		drvRsp = getStringFormFile(fileDrv351xCal);
		

		tst++;

		if (drvRsp != null && drvRsp.length()>0)
		{
			Log.d("ATD","getResFromDrv:\n"+drvRsp);

			//test to parse the integer value from string;
			String[] parseRsp = drvRsp.split(" |,|\n");
            long[] ar = new long[parseRsp.length];
			Log.d("ATD", "parse res from drv 0801_1:\n");
			
            for (int i=0; i<parseRsp.length&&i<64; i++)
            {
                ar[i] = (long)Integer.parseInt(parseRsp[i]);
				Log.d("ATD", "i="+i+", int val="+ar[i]+";\n");
				if(i>=4)
				{
					drvCoeInt += String.format("%d,", ar[i]);//(ar[i]+tst));
				}
            }
			
			/*/
			if(ar[0] == 5 && ar[1] == 129 && ar[2] == 53 && ar[3] == 56)
			{
				Log.d("ATD", "ts351x cal invalid, rollback!\n");
				newFile(strTs351xCoeFile, drvCoeInt);

				return ptRes;
			}
			/*/
			if(ar[0] == 109 && ar[1] == 1)
			{
				ptRes.x = (int)ar[2];
				ptRes.y = (int)ar[3];				
			}
			
		}
		
		return ptRes;
	}


	public boolean getTLCalResult()
	{
		//String drvRsp = getStringFormFile(fileDrv351xCalReal); //From driver
		String drvRsp = getStringFormFile(fileDrv351xCal);
		String drvCoeInt =""; //andy0716, test only,

		tst++;

		if (drvRsp != null && drvRsp.length()>0)
		{
			Log.d("ATD","getTLCalResult:\n"+drvRsp);

			//test to parse the integer value from string;
			String[] parseRsp = drvRsp.split(" |,|\n");
            long[] ar = new long[parseRsp.length];
			Log.d("ATD", "parse drv rsp 0718_1:\n");
			
            for (int i=0; i<parseRsp.length&&i<64; i++)
            {
                ar[i] = (long)Integer.parseInt(parseRsp[i]);
				Log.d("ATD", "i="+i+", int val="+ar[i]+";\n");
				if(i>=4)
				{
					drvCoeInt += String.format("%d,", ar[i]);//(ar[i]+tst));
				}
            }
			//delFile(strTs351xCoeFile);
			if(ar[0] == 5 && ar[1] == 128 && ar[2] == 53 && ar[3] == 56)
			{
				Log.d("ATD", "ts351x cal success!\n");
				//andy1013;
				drvCoeInt = "48," + drvCoeInt;
				newFile(strTs351xCoeFile, drvCoeInt);

				return true;
			}
			if(ar[0] == 5 && ar[1] == 129 && ar[2] == 53 && ar[3] == 56)
			{
				Log.d("ATD", "ts351x cal invalid, rollback!\n");
				//andy1013;
				drvCoeInt = "48," + drvCoeInt;
				newFile(strTs351xCoeFile, drvCoeInt);

				return false;
			}			
		}
		
		return false;
	}

	public void LoadCoeFromFile()
	{
		if (!fileCoeSave.exists())
			return;
		
		String strCoe = getStringFormFile(fileCoeSave); //From driver
		if (strCoe == null)
			return;
		Log.d("ATD", "Read ts351xcoe: "+strCoe);
		//andy1013;
		String[] parseIntStr = strCoe.split(" |,|\n");
		strCoe = "";
		for (int i=1; i<parseIntStr.length&&i<64; i++)
		{
			strCoe += parseIntStr[i] + ",";
		}

		//then we have to write down the coe data to driver;
		String strCoeCmd =String.format("%d,%d,%d,%d,",98, 0, 0, 14)+strCoe;
		Log.d("ATD", "strCoeCmd: "+strCoeCmd);

		//andy0717; the cal2 interface is OK
		//writeStringToFile(fileDrv351xCalReal, strCoeCmd);

		//we should get the drv interface file from the path file: /data/etc/ts351x_calpath
		String path_ts351x_calif ="";

		/*/
		//andy0718, move the following funtions to the constructer;
		if (filepath_cal2.exists())
		{
			path_ts351x_calif =getStringFormFile(filepath_cal2);
			path_ts351x_calif += "/cal2";
			File ts351x_calif = new File(path_ts351x_calif);
			writeStringToFile(ts351x_calif, strCoeCmd);			
		}
		else
		{	
			//this is the way we use hardcoded file path;
			writeStringToFile(fileDrv351xCal, strCoeCmd);
		}

		/*/

		writeStringToFile(fileDrv351xCal, strCoeCmd);

		//andy0717; we test the class file interface now;
		//this interface is not work anyway; !!!!!
		//writeStringToFile(fileTs351xCal, strCoeCmd);
	}
	public Point getPointAdc()
	{
		Log.d(TAG,"READY NEW POINT");
		Point adc = new Point(-1,-1);
		Log.d(TAG,"NEW POINT -1,-1");
		String strAdc = getStringFormFile(fileAdValue); //From driver

		Log.d(TAG, "Read String: "+strAdc);

		if(strAdc != null && strAdc.length()>0)
		{
	        int num = 0;
			char[] adcChars = strAdc.toCharArray();
	        for (int i = 0; i < adcChars.length; i++) 
			{
	            if (adcChars[i] < '0' || adcChars[i] > '9') {
					if(adcChars[i] == ',')
					{
						adc.x = num;
						num = 0;
						continue;
					}
					else
		                break;
	            }
	            num *= 10;
	            num += adcChars[i] - '0';
	        }
			adc.y = num;
 		}

		Log.d(TAG, "ADC: "+adc.toString());

		return adc;
	}
	
	public void setCalibrateAD(Point[] calibrateAD)
	{
		StringBuffer sb = new StringBuffer(100);
		String substr;

		for(int i = 0; i < 5; i++)
		{
			substr = String.format("%04d,%04d", calibrateAD[i].x, calibrateAD[i].y);
			Log.d(TAG, substr);
			if(i < 4)
				substr += ",";
			sb.append(substr);
		}
		Log.d( TAG, "Write string: "+ sb.toString() );
		
		writeStringToFile(fileAdCalibrate, sb.toString());
	}
}

//package com.telink.tscalibration;
package com.telink.tscalibrationv2;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.os.SystemProperties;
import android.content.res.Configuration;
import android.util.Log;
import android.os.Message;
//import Android.util.DisplayMetrics;


public class TSCalibration extends Activity {
    /** Called when the activity is first created. */
	private final String TAG="TSCalibration";

    private AppToDriver mATD = new AppToDriver();
	private int currTouch = 0;
	private Point[] calibrateAD = new Point[7];
	private Point curTouchAdc = null;// new Point(0,0);
	private Point pres0 = new Point(0, 0);
	 // (800, 480); (250, 250);//default value if all methods are failed;
	private Point pres = new Point(250, 250);
	private Point pdel = new Point(10, 10);

	/*/
	private Point p1 = new Point(50, 50);      //o;
	private Point p2 = new Point(800-50, 50);  //a;
	private Point p3 = new Point(50, 480-50);	//c;
	private Point p4 = new Point(800-50, 480-50); //b;
	/*/
	
	private Point p1 = new Point(0, 0);
	private Point p2 = new Point(0, 0);
	private Point p3 = new Point(0, 0);
	private Point p4 = new Point(0, 0);
	
	public ProgressDialog pd;
	
	private final int TOUCH_TOP_LEFT     = 0;
	private final int TOUCH_TOP_RIGHT    = 1;
	private final int TOUCH_BOTTOM_LEFT  = 2;
	private final int TOUCH_BOTTOM_RIGHT = 3;
	private final int TOUCH_CENTER       = 4;
	private final int TOUCH_CHECK_END	= 7; //5; o, a, c, oa, oc, touch_chk and finish; andy0714, add "b" finally;
	
	private TouchView touchView;
    private ImageView notification;

	//andy0802;	
	private Display display;
    
    private String locale = null;
	//public final int REFRESH =100;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

		//andy1023, we always get the res_LCD from android system; 
		//pres0 =mATD.getResFromDrv();  //andy1023, depreciate the mehtod to get res via driver;
		if(pres0.x < 100 || pres0.y <100)
		{
			display = ((WindowManager) getSystemService(WINDOW_SERVICE)).getDefaultDisplay();
			pres0.x = display.getWidth();
			pres0.y = display.getHeight();
		}

		if(pres0.x > 100 && pres0.y > 100)
			pres = pres0;
		
		p1.x =p3.x =pres.x/10 -10;
		p1.y =p2.y =pres.y/10 -10;

		p2.x =p4.x =9*pres.x/10 -10;
		p3.y =p4.y =9*pres.y/10 -10;
		
		
		/*/ start of andy1023;
		//new Thread(new GameThread()).start();

		rfhdl =new RefHandler();

		new Thread(new TestThread()).start();
	
		///////////////////////////
		/*/ //andy1023; remove the legacy testing and gaming threads; which are used to refresh the view;


        setupViews();

    }

	
    private void setupViews()
    {
    	String substr;
        //full screen
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
	    WindowManager.LayoutParams.FLAG_FULLSCREEN);
		//no title
        requestWindowFeature(Window.FEATURE_NO_TITLE);         
        setContentView(R.layout.main);

		
        
    	//initialize
		for (int i = 0; i < TOUCH_CHECK_END; i++)
			calibrateAD[i] = new Point(-1, -1);
			
	  	//get Configuration
        final Configuration configuration = getResources().getConfiguration();
        locale = configuration.locale.toString();

		touchView = (TouchView) findViewById(R.id.TouchView);
		touchView.addTouchPoint(p1);
		touchView.invalidate();
		//andy0730 add action to show point o here;
		
		notification = (ImageView) findViewById(R.id.notification);
		
		/*/  andy1023; remove the following code;
		try {
			Thread.sleep(100);
			} catch (InterruptedException e) {
			return; //false;
		  }
		/*/    //end of andy1023;

		substr = String.format("%d,%d,%d,%d,",67, 0, 16, 18);
		mATD.writeStringToFile(mATD.fileDrv351xCal, substr);

		
        if(locale.equals("zh_CN")){
        	notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification_zh));
        }else{
        	notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification));
        }
    }
     @Override
     protected void onDestroy() {
         super.onDestroy();
		 
         System.exit(0);
         finish();
     }

	//andy1023, try to handle the issue when calibration app is interrupted by others;
	@Override	
	public void onWindowFocusChanged(boolean hasFocus) {  
		// TODO Auto-generated method stub
		String cmdstr;
		if(hasFocus == false) //interrupted by others;
		{
			Log.d("TSCal", "Cal_app lose focus;\n");
			cmdstr = String.format("%d,%d,%d,%d,",107, 35, 16, 18);
			mATD.writeStringToFile(mATD.fileDrv351xCal, cmdstr);
		}
		else  //resume from interrupt;
		{
			Log.d("TSCal", "Cal_app re-gain focus;\n");
			touchView.removeAllTouchPoint();
			touchView.addTouchPoint(p1);
			//touchView.addTouchPoint(p2);
			touchView.invalidate();		
												
					    if(locale.equals("zh_CN")){
						notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification_zh));
					}else{
			        	notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification));
			        }
					    currTouch=0;
						cmdstr = String.format("%d,%d,%d,%d,",67, 0, 16, 18);
						//substr = String.format("%d,%d,%d,%d,",108, 0, 16, 18);
						mATD.writeStringToFile(mATD.fileDrv351xCal, cmdstr);			
		}
		super.onWindowFocusChanged(hasFocus);  
	} 

	 
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		StringBuffer sb = new StringBuffer(100);
		String substr;
	
		if(event.getAction() == MotionEvent.ACTION_DOWN
				         && currTouch != TOUCH_CHECK_END){		

			//andy0717, the following two methods has been verified with driver;
			/*/
			mATD.getTLCalResult();
			mATD.LoadCoeFromFile(); 
			/*/

			//Log.d("ATD", "i="+i+", int val="+ar[i]+";\n");
			Log.d("TSCal", "onTouchEvent: new evt, sys res_LCD is: x="+pres0.x+", y="+pres0.y+";\n");

			++currTouch;
			switch(currTouch) {
			case 1:	
				touchView.removeAllTouchPoint();

				//mOpenHandler_andy.post(mOpeningRun_andy);
			
				touchView.addTouchPoint(p2);
				touchView.invalidate();

				//mOpenHandler_andy.post(mOpeningRun_andy);

				//andy0714, we show a first then wait 1.5 seconds for the user to move finger
				//before we issue the command to the driver;

				/*/
		      		try {
			        Thread.sleep(50);   //(2000); //test!!!!
					} catch (InterruptedException e) {
			        return false;
			      }
				/*/	
					
				//substr = String.format("%d,%d,%d,%d,",67, 01, 16, 18); // new cmd_id, 108;
				substr = String.format("%d,%d,%d,%d,",108, 01, 16, 18); //
				mATD.writeStringToFile(mATD.fileDrv351xCal, substr);
				break;

				
				//andy0714, add special step to "cal b", it is NOT necessary for the chip;
				//but it is used by the driver to double check the user's input,
				//to avoid a malfunction;
				
				//clear a then show b;	
				case 2: 	
				touchView.removeAllTouchPoint();					
				touchView.addTouchPoint(p4);
				touchView.invalidate();
				
					
				//andy0714, we show b first then wait 1.5 seconds for the user to move finger
				//before we issue the command to the driver;
				try {
					Thread.sleep(50);
					} catch (InterruptedException e) {
					return false;
				  }
									

				//note: point b is very special; (53)d =0x35;
				//substr = String.format("%d,%d,%d,%d,",67, 53, 16, 18);
				substr = String.format("%d,%d,%d,%d,",108, 53, 16, 18);
				mATD.writeStringToFile(mATD.fileDrv351xCal, substr);
				break;
				
			case 3:			
				touchView.removeAllTouchPoint();
				touchView.addTouchPoint(p3);
				touchView.invalidate();
				try {
					Thread.sleep(50);
					} catch (InterruptedException e) {
					return false;
				  }
				
				//substr = String.format("%d,%d,%d,%d,",67, 02, 16, 18);//"c" calID is 2;
				substr = String.format("%d,%d,%d,%d,",108, 02, 16, 18);
				mATD.writeStringToFile(mATD.fileDrv351xCal, substr);
				break;
				
			case 4:				
				touchView.removeAllTouchPoint();
				touchView.invalidate();
				try {
					Thread.sleep(50);
					} catch (InterruptedException e) {
					return false;
				  }				

				//for telink tlsc351x series, step3 is to cal oa, 
				//i.e fw need to snapshot the adc values when both o and a are pressed.
				//so show both "o" and "a" at step3;
				

				//note: for dual touch, we don't have to wait;				
				touchView.addTouchPoint(p2);
				touchView.addTouchPoint(p1);
				touchView.invalidate();

				//substr = String.format("%d,%d,%d,%d,",67, 03, 16, 18);
				substr = String.format("%d,%d,%d,%d,",108, 03, 16, 18);
				mATD.writeStringToFile(mATD.fileDrv351xCal, substr);				

				break;
				
			case 5:				
				touchView.removeAllTouchPoint();
				touchView.invalidate();

		      	try {
			        Thread.sleep(50);
					} catch (InterruptedException e) {
			        return false;
			      }
					
				//for telink tlsc351x series, step4 is to cal oc, 
				//i.e fw need to snapshot the adc values when both o and c are pressed.
				//so show both "o" and "c" at step4;
				

				touchView.addTouchPoint(p1);
				touchView.addTouchPoint(p3);
				touchView.invalidate();

				//substr = String.format("%d,%d,%d,%d,",67, 04, 16, 18);
				substr = String.format("%d,%d,%d,%d,",108, 04, 16, 18);
				mATD.writeStringToFile(mATD.fileDrv351xCal, substr);					
					
				break;

			case 6:				
				touchView.removeAllTouchPoint();
				touchView.invalidate();

				substr = String.format("%d,%d,%d,%d,",68, 05, 16, 18);
				mATD.writeStringToFile(mATD.fileDrv351xCal, substr);				
			
				if(locale.equals("zh_CN")){
					notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification_done_zh));
				}else{
					notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification_done));
				}
				break;
			} 
			
			if (currTouch == TOUCH_CHECK_END)
			{
					//if (mATD.doCheckCalibration() == true)
					if (mATD.getTLCalResult() == true)
					{
						// save this 5 point AD value to file
						//saveToPreference(); //no necessary for ts351x;

						pd = ProgressDialog.show(TSCalibration.this, 
									getString(R.string.adjust_touch_screen_finish), 
									getString(R.string.adjust_touch_screen_finish_data), true, false);
									
						mOpenHandler.postDelayed(mOpeningRun, 1000);//andy, wait for 1sec then call mOpeningRun
					}
					else
					{
                        int queryForce=Integer.parseInt(SystemProperties.get("app.tscalibration.force","0"));
                        if(queryForce == 1) {
						
						touchView.addTouchPoint(p1);
						//touchView.addTouchPoint(p2);
						touchView.invalidate();
							
						    if(locale.equals("zh_CN")){
							notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification_zh));
						}else{
				        	notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification));
				        }
                            currTouch=0;
							//andy0717;
							try {
								Thread.sleep(50);
								} catch (InterruptedException e) {
								return false; //false;
							  }
							//String substr;
							substr = String.format("%d,%d,%d,%d,",67, 0, 16, 18);
							//substr = String.format("%d,%d,%d,%d,",108, 0, 16, 18);
							mATD.writeStringToFile(mATD.fileDrv351xCal, substr);

							
                        } else {
                            AlertDialog.Builder builder = new AlertDialog.Builder(TSCalibration.this);
						    builder.setIcon(R.drawable.alertdialog)
                               .setTitle(R.string.alert_dialog)
						       .setMessage(R.string.alert_dialog_message)
                               .setCancelable(false)
						       .setPositiveButton(getString(R.string.home_screen), new DialogInterface.OnClickListener() {
						           public void onClick(DialogInterface dialog, int id) {
						               onDestroy(); 
						        	   TSCalibration.this.finish();
						        }
						    })
						       .setNegativeButton(getString(R.string.try_again), new DialogInterface.OnClickListener() {
						           public void onClick(DialogInterface dialog, int id) {
						                dialog.cancel();

						touchView.addTouchPoint(p1);
						//touchView.addTouchPoint(p2);
						touchView.invalidate();									
									

									
								    if(locale.equals("zh_CN")){
        							notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification_zh));
        						}else{
						        	notification.setBackgroundDrawable(getResources().getDrawable(R.drawable.notification));
						        }
								        currTouch=0;
									//andy0714;
							      	try {
								        Thread.sleep(50);
										} catch (InterruptedException e) {
								        return; //false;
								      }
									String substr;
									substr = String.format("%d,%d,%d,%d,",67, 0, 16, 18);
									//substr = String.format("%d,%d,%d,%d,",108, 0, 16, 18);
									mATD.writeStringToFile(mATD.fileDrv351xCal, substr);								        
						        }
						    });
						    AlertDialog alert = builder.create();
						    alert.show();
						}
						
					}
			}
   		}

	else if(event.getAction() == MotionEvent.ACTION_UP
				         && currTouch != TOUCH_CHECK_END)
	{
		//substr = String.format("%d,%d,%d,%d,",67, 04, 16, 18);
		//mATD.writeStringToFile(mATD.fileDrv351xCal, substr);
		//substr ="";
		if(currTouch < 2 )
		{
			substr = String.format("%d,%d,%d,%d,",67, currTouch, 16, 18);
		}
		else if(currTouch == 2 )
		{
			substr = String.format("%d,%d,%d,%d,",67, 53, 16, 18);
		}
		else  //if(currTouch >2)
		{
			substr = String.format("%d,%d,%d,%d,",67, currTouch-1, 16, 18);
		}

		/*/
		//wait for 0.5s here;
		try {
			Thread.sleep(100);   // Thread.sleep(1000);
			} catch (InterruptedException e) {
			return false;
		  }
		/*/
	//////!!!!!!!!!!!!!!!!!! andy1023, remove the code here; change from 1000 -> 100;
			

		/*/
		if(currTouch == 0)
		{
			touchView.addTouchPoint(p1);
		}
		if(currTouch == 1)
		{
			touchView.addTouchPoint(p1);
		}
		if(currTouch == 2)
		{
			touchView.addTouchPoint(p1);
		}
		if(currTouch == 3)
		{
			touchView.addTouchPoint(p1);
		}
		if(currTouch == 4)
		{
			touchView.addTouchPoint(p1);
		}
		if(currTouch == 5)
		{
			touchView.addTouchPoint(p1);
		}
		touchView.invalidate();
		/*/

		if(currTouch < 6 && currTouch > 0)
		{
			mATD.writeStringToFile(mATD.fileDrv351xCal, substr);
		}
		
	}
   		return true;

		
	}
	@Override
	public void onConfigurationChanged(Configuration newConfig) 
	{
		if(newConfig.orientation==Configuration.ORIENTATION_PORTRAIT)
		{    	
			System.out.println("ORIENTATION_PORTRAIT");
		}
	
		if(newConfig.orientation==Configuration.ORIENTATION_LANDSCAPE)
		{	
			System.out.println("ORIENTATION_LANDSCAPE");
		}
		super.onConfigurationChanged(newConfig);
	}

    private void saveToPreference()
	{
        SharedPreferences prefCalibrate = getSharedPreferences(mATD.TOUCH_CHECK_VALUE, 0);
		SharedPreferences.Editor ed = prefCalibrate.edit();

		for (int i = 0; i < (TOUCH_CHECK_END-1); i++)
		{
			ed.putInt(mATD.strTouch[i*2+0], calibrateAD[i].x);
			ed.putInt(mATD.strTouch[i*2+1], calibrateAD[i].y);
		}
		
		ed.commit();
	}
    
	Handler mOpenHandler = new Handler();	

	Runnable mOpeningRun = new Runnable() {
		public void run() { 
			mOpenHandler.removeCallbacks(mOpeningRun);
			pd.dismiss();
            onDestroy();
        }
	};

	
}

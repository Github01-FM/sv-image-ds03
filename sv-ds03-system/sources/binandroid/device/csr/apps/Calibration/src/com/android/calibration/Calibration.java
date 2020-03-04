
package com.android.calibration;

import android.util.Slog;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.TextView;
import android.widget.Button;
import android.widget.ProgressBar;
import android.view.View;
import android.view.View.OnClickListener;
import com.android.calibration.NativeCalibration;

public class Calibration extends Activity
{
    	static final String TAG = "TP-calibration";
	private ProgressBar barProgress;
	private TextView txtMessage;
	private Button btnStart;	
	private OnClickListener lsnStart;
	private Button btnHelp;
	private OnClickListener lsnHelp;
	private String strMessage = "";
	private int iProgress = 0;
	private boolean bSuccess = false;
	private boolean bStop = false;
	private int iRet = 0;
	private int iFptr = -1;
	private static final int CNTF = 0X100;
	private static final int CNTS = 0x101;
	private static final int NEXT = 0x102;
	private static final int DOSU = 0x103;
	private static final int CALI = 0x104;
	private static final int STOP = 0x105;

	private static final int REST = 20;
	private static final int MAXP = 120;
	private static final int MAXX = REST + MAXP;
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);			
		lsnStart = new OnClickListener()  
		{
			public void onClick(View v)
			{				
				if (iFptr != -1)
				{
					txtMessage.setGravity(0x01);
					barProgress.setVisibility(ProgressBar.VISIBLE);	
					barProgress.setProgress(0);
					runThread();
					btnStart.setEnabled(false);
					
				}
			}
		};
		
		lsnHelp = new OnClickListener()  
		{
			public void onClick(View v)
			{	
				barProgress.setVisibility(ProgressBar.INVISIBLE);
				txtMessage.setGravity(0x03);
				String str = "If the pop-up \"Open device fail:\"\n" 
					+ " 1> Check the \"/dev/pixcir_tangoc_ts0\" exists.\n" 
					+ " 2> If the user has read and write permission.";				
				txtMessage.setText(str);				
			}
		};
		
		setContentView(R.layout.main);
		btnStart = (Button) findViewById(R.id.BTN_START);
		btnStart.setOnClickListener(lsnStart);
		btnHelp = (Button) findViewById(R.id.BTN_HELP);
		btnHelp.setOnClickListener(lsnHelp);
		txtMessage = (TextView) findViewById(R.id.CALI_MSG);
		barProgress = (ProgressBar) findViewById(R.id.CALI_PROGRESS);
		barProgress.setIndeterminate(false);
		barProgress.setMax(MAXP);
	}  

	@Override
	public void onStart()
	{
		super.onStart();
		String dev = "/dev/pixcir_tangoc_ts0";
		if ((iFptr = NativeCalibration.open(dev)) == -1)
		{
			new MessageBox(Calibration.this, "Warning", "Open device fail.");
		}	
	}

	@Override
	public void onDestroy()                                                                    
	{
		super.onDestroy();

		if (iFptr != -1)
		{
			NativeCalibration.close();			
		}
	}

	private void startCountdown()
	{
		txtMessage.setText(strMessage);
		barProgress.setProgress(iProgress);
		strMessage = "Please don't touch screen.\n" + "Countdown ";
		txtMessage.setText(strMessage + "2 s");
	}

	private void startCalibrate()
	{
		strMessage += "0 s\n" + "Calibrate start.\n";
		txtMessage.setText(strMessage);
	}

	private void doCalibrate()
	{
		NativeCalibration.ioctl(NativeCalibration.CALIBRATION_FLAG);
		byte[] bytes = new byte[2];		
		bytes[0] = 0x3a;
		bytes[1] = 0x03;
		String str = new String(bytes);
		iRet = NativeCalibration.write(str);
		Slog.d(TAG, "=======run to here===== iRet ="+iRet);
	}

	private void getCalibrate()
	{
		if (iRet == 0)
		{
			strMessage += "Calibrate success.\n";
		}
		else
		{
			strMessage += "Calibrate fail.\n";
		}
		txtMessage.setText(strMessage);
	}

	private void runThread()
	{
		new Thread(new Runnable()
		{
			public void run()
			{
				for (int count = 1; count <= MAXX; count++)
				{
					try
					{
						Thread.sleep(100);
						Message msg = new Message();
						if (bStop || bSuccess)
						{
							iProgress = MAXP;
							count = MAXX;		
						}
						else if (count > REST)
						{
							iProgress = count - REST;
							
						}
						switch (count)
						{
						case 1:
							msg.what = CNTF;
							break;
						case 10:
							msg.what = CNTS;
							break;
						case 20:
							msg.what = DOSU;
							break;
						case 21:
							msg.what = CALI;
							break;
						case MAXX:
							msg.what = STOP;
							break;
						default:
							msg.what = NEXT;
							break;
						}
					Calibration.this.mHandler.sendMessage(msg);
					}
					catch (Exception e)
					{
						e.printStackTrace();
					}
				}
			}
		})
		{
		}.start();
	}

	private Handler mHandler = new Handler() 
	{
		public void handleMessage(Message msg)
		{
			switch (msg.what)
			{
			case CNTF:
				if (!Thread.currentThread().isInterrupted())
				{						
					startCountdown();
				}
				break;
			case CNTS:
				if (!Thread.currentThread().isInterrupted())
				{
					txtMessage.setText(strMessage + "1 s");
				}
				break;
			case DOSU:
				if (!Thread.currentThread().isInterrupted())
				{
					barProgress.setProgress(iProgress);
					startCalibrate();
				}
				break;
			case CALI:
				if (!Thread.currentThread().isInterrupted())
				{
					barProgress.setProgress(iProgress);
					doCalibrate();
				}
				break;
			case NEXT:
				if (!Thread.currentThread().isInterrupted())
				{
					barProgress.setProgress(iProgress);
				}
				break;
			case STOP:
				if (!Thread.currentThread().isInterrupted())
				{
					barProgress.setProgress(MAXP);
					btnStart.setEnabled(true);
					getCalibrate();
				}
				Thread.currentThread().interrupt();				
				break;
			}
			super.handleMessage(msg);
		}
	};
}

class MessageBox
{
	MessageBox(final Activity activity, String title, String content)
	{
		new AlertDialog.Builder(activity).setTitle(title).setMessage(content)
				.setPositiveButton("Ok", new DialogInterface.OnClickListener()
				{
					public void onClick(DialogInterface dialog, int whichButton)
					{
						activity.finish();
					}
				}).show();
	}
}


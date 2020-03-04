//package com.telink.tscalibration;
package com.telink.tscalibrationv2;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.util.AttributeSet;
import android.view.View;

public class TouchView extends View {
	
	private List<Point> list = new ArrayList<Point>();
	private Bitmap bmTouchPoint = null;

	public TouchView(Context context, AttributeSet attrs) {
		super(context, attrs);
		
		bmTouchPoint = BitmapFactory.decodeResource(
				context.getResources(),
				R.drawable.touch_point);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);

		Paint paint = new Paint();

		for(int i = 0; i < list.size(); i++) {
			Point p = list.get(i);
			canvas.drawBitmap(bmTouchPoint, p.x, p.y, paint);
		}
	}

	public void addTouchPoint(Point p) {
		if (!list.contains(p)){
			list.add(p);
		}
	}
	
	public void removeAllTouchPoint() {
		list.clear();
	}
}

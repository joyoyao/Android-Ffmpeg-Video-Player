package com.example.chengang.myapplication;

import android.graphics.Bitmap;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ImageView;
import android.widget.TextView;


public class MainActivity extends ActionBarActivity {
    private Bitmap mBitmap;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView view = (TextView) findViewById(R.id.mytext);

        mBitmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888);
        view.setText(this.getStringFromNative("/sdcard/1.flv", mBitmap));

        ImageView i = (ImageView)findViewById(R.id.frame);
        i.setImageBitmap(mBitmap);
//        getStringFromNative();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public native String getStringFromNative(String url, Bitmap bitmap);
    static {
        System.loadLibrary("swresample-1");
        System.loadLibrary("avutil-54");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("ovsplayer");
    }
}

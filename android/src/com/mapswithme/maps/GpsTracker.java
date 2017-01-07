package com.mapswithme.maps;

import android.graphics.Bitmap;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.util.Random;

/**
 * Created by andika on 26/11/16.
 */

public class GpsTracker {

    public enum State {
        Empty,
        Started,
        Paused,
        Stopped
    }

    public static native void startGpxPlayback(String gpx);

    public static native void stopGpxPlayback();

    public static native void startTracking();

    public static native void pauseTracking();

    public static native void stopTracking(boolean cancel);

    public static native void clearTracking();

    public static native void setTrackerInfoListener(Framework.TrackerInfoListener listener);

    public static native State getState();
}

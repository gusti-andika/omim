package com.mapswithme.maps;

/**
 * Created by andika on 26/11/16.
 */

public class GpsTracker {

    public static native void startGpxPlayback(String gpx);

    public static native void stopGpxPlayback();

    public static native void startTracking();

    public static native void stopTracking(boolean cancel);

    public static native void clearTracking();
}

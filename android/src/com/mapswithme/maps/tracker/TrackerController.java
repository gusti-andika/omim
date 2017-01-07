package com.mapswithme.maps.tracker;

import android.app.Activity;
import android.content.Intent;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.mapswithme.maps.Framework;
import com.mapswithme.maps.GpsTracker;
import com.mapswithme.maps.R;
import com.mapswithme.util.UiUtils;
import com.mapswithme.util.concurrency.UiThread;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Timer;

/**
 * Created by andika on 30/11/16.
 */

public class TrackerController implements Framework.TrackerInfoListener {

    private TextView avgSpeedText;
    private TextView distanceText;
    private TextView timeText;

    private TextView startPauseButton;
    private TextView stopButton;

    private View parent;

    private Timer timer;

    private TrackerMenu menu;

    private Activity activity;

    private boolean started;

    private boolean nativeInitialized;

    public static class TrackerInfo {
        double avgSpeed;
        double lastSpeed;
        double distance;

        public TrackerInfo(double avgSpeed, double lastSpeed, double distance) {
            this.avgSpeed = avgSpeed;
            this.lastSpeed = lastSpeed;
            this.distance = distance;
        }
    }

    private final View.OnClickListener buttonsListener = new View.OnClickListener() {

        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.pauseBtn:

                    if (GpsTracker.getState() == GpsTracker.State.Paused) {
                        start();
                        startPauseButton.setText("PAUSE");
                    } else if (GpsTracker.getState() == GpsTracker.State.Started) {
                        pause();
                        startPauseButton.setText("START");
                    }

                    break;
                case R.id.stopBtn:
                    stop();
                    //Framework.nativeCaptureMapPNG();
                    //TODO: go to TrackerActivityReport fragment
                    break;
                case R.id.captureBtn:
                    //Framework.nativeCaptureMapPNG();
                    break;
            }
        }
    };

    public TrackerController(Activity activity, View menuView, View controllerView) {
        parent = controllerView;
        this.activity = activity;
        initTextItems(controllerView);
        initButtons(controllerView);

        menu = new TrackerMenu(menuView, this);
    }

    @Override
    public void onTrackerInfoUpdated(final TrackerInfo info) {
        UiThread.run(new Runnable() {

            @Override
            public void run() {
                avgSpeedText.setText(String.format("%.2f m/s", info.avgSpeed));
                //laSpeedText.setText(String.format("%d m/s", info.avgSpeed));
                distanceText.setText(String.format("%.2f m", info.distance));
            }
        });
    }

    public TrackerMenu getMenu() {
        return menu;
    }

    private void initNative() {
        if (nativeInitialized)
            return;

        GpsTracker.setTrackerInfoListener(this);
        nativeInitialized = true;
    }

    private void initTextItems(View root) {
        View parent = root.findViewById(R.id.avg_speed);
        ((TextView)parent.findViewById(R.id.caption)).setText("speed");
        avgSpeedText = (TextView) parent.findViewById(R.id.content);

        parent = root.findViewById(R.id.time);
        ((TextView)parent.findViewById(R.id.caption)).setText("duration");
        timeText = (TextView) parent.findViewById(R.id.content);

        parent = root.findViewById(R.id.distance);
        ((TextView)parent.findViewById(R.id.caption)).setText("distance");
        distanceText = (TextView) parent.findViewById(R.id.content);
    }

    private void initButtons(View root) {
        startPauseButton = (TextView) root.findViewById(R.id.pauseBtn);
        startPauseButton.setOnClickListener(buttonsListener);

        stopButton = (TextView) root.findViewById(R.id.stopBtn);
        stopButton.setOnClickListener(buttonsListener);

        root.findViewById(R.id.captureBtn).setOnClickListener(buttonsListener);
    }

    public void start() {
        initNative();
        started = true;
        GpsTracker.startTracking();
    }

    public void pause() {
        GpsTracker.pauseTracking();
    }

    public void stop() {
        GpsTracker.stopTracking(false);
    }

    public void open() {
        UiUtils.show(parent);
    }

    public void close() {
        UiUtils.hide(parent);
    }

}

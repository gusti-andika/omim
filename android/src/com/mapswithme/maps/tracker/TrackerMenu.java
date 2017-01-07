package com.mapswithme.maps.tracker;

import android.view.View;

import com.mapswithme.maps.GpsTracker;
import com.mapswithme.maps.R;
import com.mapswithme.util.UiUtils;

/**
 * Created by andika on 30/11/16.
 */

public class TrackerMenu {

    View menu;

    TrackerController controller;

    public TrackerMenu(View root, final TrackerController controller) {
        this.menu = root;
        this.controller = controller;

        View goBtn = root.findViewById(R.id.goBtn);
        View gpxBtn = root.findViewById(R.id.gpxBtn);

        View.OnClickListener btnListeners = new View.OnClickListener() {

            @Override
            public void onClick(View v) {

                switch(v.getId()) {
                    case R.id.goBtn:
                        close(new Runnable() {
                            @Override
                            public void run() {
                                controller.start();
                                controller.open();
                            }
                        });
                        break;
                    case R.id.gpxBtn:
                        GpsTracker.startGpxPlayback("/mnt/sdcard/20161120.gpx");
                        break;
                }


            }
        };

        goBtn.setOnClickListener(btnListeners);
        gpxBtn.setOnClickListener(btnListeners);
    }

    public void open() {
        UiUtils.show(menu);
    }

    public void close(Runnable run) {
        //TODO::???
        UiUtils.hide(menu);
        run.run();
    }
}

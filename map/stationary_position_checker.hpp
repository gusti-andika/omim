#pragma once

#include "base/timer.hpp"
#include "platform/location.hpp"
#include "std/unique_ptr.hpp"


class StationaryPositionChecker
{
public:


    using TFuncCallback=function<void(location::GpsInfo const &info)>;

    StationaryPositionChecker();

    static StationaryPositionChecker & Instance();

    void CheckPositionStationary(location::GpsInfo const &info);

    void SetCallbacks(TFuncCallback pauseCb, TFuncCallback resumeCb){
        m_pauseCallback = pauseCb;
        m_resumeCallback = resumeCb;
    }

    bool IsPositionStationary(){ return m_isPause;}
private:


    TFuncCallback m_pauseCallback;
    TFuncCallback m_resumeCallback;
    my::Timer m_timer;
    unique_ptr<location::GpsInfo> m_lastInfo;
    double m_StartTime = -1;
    bool m_isPause = false;
};


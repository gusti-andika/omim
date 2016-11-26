#include "stationary_position_checker.hpp"
#include "geometry/distance_on_sphere.hpp"
#include "base/logging.hpp"

int const kStationaryDistance = 15; //stationary position distance threshold in metres
int const kStationaryCheckDuration = 30;

void StationaryPositionChecker::CheckPositionStationary(const location::GpsInfo & info) {
    //LOG(LDEBUG, ("GpsInfo: ", info));

    if (!m_lastInfo) {
        m_lastInfo.reset(new location::GpsInfo(info));
        return;
    }

    double distance = ms::DistanceOnEarth(m_lastInfo->m_latitude, m_lastInfo->m_longitude,
                                          info.m_latitude, info.m_longitude);
    double tstampSpan = info.m_timestamp - m_lastInfo->m_timestamp;
    //LOG(LDEBUG, ("Distance: ", distance, ", TstampSpan:", tstampSpan));

    if (!m_isPause && distance <= kStationaryDistance) {
        if (m_StartTime > 0) {
            double elapsed = m_timer.ElapsedSeconds() - m_StartTime;
            if (elapsed >= kStationaryCheckDuration || tstampSpan >= kStationaryCheckDuration) {
                m_StartTime = -1;
                m_isPause = true;
                if (m_pauseCallback)
                    m_pauseCallback(info);
            }

        } else {
            m_StartTime = m_timer.ElapsedSeconds();
        }
    } else {
        m_StartTime = -1;
        if (m_isPause && distance > kStationaryDistance) {
            m_isPause = false;
            if (m_resumeCallback)
                m_resumeCallback(info);
        }
    }

    m_lastInfo.reset(new location::GpsInfo(info));;
}

StationaryPositionChecker::StationaryPositionChecker()
    : m_StartTime(0)
    , m_isPause(false)
{

}

StationaryPositionChecker & StationaryPositionChecker::Instance() {
    static StationaryPositionChecker instance;
    return instance;
}


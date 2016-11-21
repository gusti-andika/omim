#pragma once

#include "map/framework.hpp"
#include "gpx_parser.hpp"
#include "std/thread.hpp"
#include "base/timer.hpp"

namespace location{
class GpsInfo;
}

namespace gpx {
class GpxPlayback {
public:

    enum class State{
        STOPPED,
        PLAYING
    };

    GpxPlayback(const string & filePath);
    ~GpxPlayback();
    void Play();
    void Stop();
    State GetState(){
        return m_state;
    }

    using TOnLocationUpdateCallback = std::function<void(location::GpsInfo const & info)>;

    void SetOnLocationUpdateCallback(TOnLocationUpdateCallback cb) {
        m_callback = cb;
    }

private:
  thread m_thread;
  bool m_threadExit;
  gpx::Gpx m_gpx;
  my::Timer m_timer;
  State m_state;
  TOnLocationUpdateCallback m_callback;
};

}

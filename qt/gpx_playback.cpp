#include "gpx_playback.hpp"
#include "coding/file_reader.hpp"
#include "base/thread.hpp"
#include "map/gps_tracker.hpp"
#include "coding/parse_xml.hpp"

namespace gpx {



GpxPlayback::GpxPlayback(const string &filePath, Framework & framework)
    : m_threadExit(false)
    , m_framework(framework)
{
    FileReader reader(filePath);
    ReaderSource<FileReader> source(reader);
    GpxParser parser(m_gpx);
    ParseXML(source, parser, true);
    //GpsTracker::Instance().SetEnabled(false);
    //m_framework.ConnectToGpsTracker();
}

void GpxPlayback::Play() {
    if (m_gpx.isTrackEmpty())
        return;

      int count = 0;
    m_thread=thread([this, &count](){
         vector<GpsInfoT> track2 = m_gpx.getTrack();
          LOG(LDEBUG, ("track size --> ", track2.size()));
        for(vector<GpsInfoT>::iterator i = track2.begin(); i < track2.end() && !m_threadExit; ++i) {
            //LOG(LDEBUG, (++count, "--> tstamp:",i->m_timestamp , ",lat:", i->m_latitude, ", lon:", i->m_longitude));
            m_callback(*i);
            threads::Sleep(1000);
        }
    });
}

void GpxPlayback::Stop() {
    if (m_state == State::STOPPED)
        return;

    m_threadExit = true;
    if (m_thread.joinable())
        m_thread.join();
    m_state = State::STOPPED;
}


GpxPlayback::~GpxPlayback() {
    m_threadExit = true;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}



}

#include "gpx_playback.hpp"
#include "coding/file_reader.hpp"
#include "base/thread.hpp"
#include "map/gps_tracker.hpp"
#include "coding/parse_xml.hpp"

namespace gpx {

GpxPlayback::GpxPlayback(const string &filePath)
    : m_threadExit(false)
{
    FileReader reader(filePath);
    ReaderSource<FileReader> source(reader);
    GpxParser parser(m_gpx);
    ParseXML(source, parser, true);
}

void GpxPlayback::Play() {
    if (m_gpx.isTrackEmpty())
        return;

        int count = 0;
        m_thread=thread(
            [this, &count]()
            {
                vector<GpsInfoT> track2 = m_gpx.getTrack();
                for(vector<GpsInfoT>::iterator i = track2.begin(); i < track2.end() && !m_threadExit; ++i) {
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

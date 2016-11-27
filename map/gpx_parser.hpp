#pragma once
#include "platform/location.hpp"
#include "std/vector.hpp"
#include "std/unique_ptr.hpp"

namespace gpx {

using GpsInfoT = location::GpsInfo;

class Gpx {
public:
    Gpx():m_baseTimestamp(0){

    }

    void addGpsInfo(GpsInfoT info) {
        if (m_track.empty())
            m_baseTimestamp = info.m_timestamp;
        m_track.push_back(info);
    }

    vector<GpsInfoT> const & getTrack() const {
        return m_track;
    }

    void clearTrack() {
        m_track.clear();
    }

    bool isTrackEmpty() {
        return m_track.empty();
    }

private:
    vector<GpsInfoT> m_track;
    double m_baseTimestamp;

};


class GpxParser {

    Gpx & m_gpx;
    GpsInfoT _temp;
    vector<string> _tags;

    string const & getTagFromEnd(size_t i);

public:
    GpxParser(Gpx & gpx);
    bool Pop(string const & name);
    bool Push(string const & name);
    void AddAttr(string const & attr, string const & value);
    void CharData(string data);

};

}


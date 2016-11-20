#include "gpx_parser.hpp"
#include "base/string_utils.hpp"
#include "base/timer.hpp"

namespace gpx {

    const string GPX_LAT = "lat";
    const string GPX_LON = "lon";
    const string GPX_ELE = "ele";
    const string GPX_TIME = "time";
    const string GPX_SPEED = "speed";
    const string GPX_TRKSEG = "trkseg";
    const string GPX_TRKPT = "trkpt";

    GpxParser::GpxParser(Gpx & gpx):m_gpx(gpx) {

    }

    bool GpxParser::Push(string const &name) {
        if (name == GPX_TRKPT){
            _temp = GpsInfoT();
            _temp.m_source = location::EGPX;
        }

        _tags.push_back(name);
        return true;
    }

    bool GpxParser::Pop(string const &name) {
        if (name == GPX_TRKPT)
            m_gpx.addGpsInfo(_temp);

        _tags.pop_back();
        return true;
    }

    void GpxParser::AddAttr(string const &attr, string const &value) {

        if (!value.empty() && attr == GPX_LAT && this->getTagFromEnd(0) == GPX_TRKPT) {
            strings::to_double(value, _temp.m_latitude);
        } else if (!value.empty() && attr == GPX_LON  && this->getTagFromEnd(0) == GPX_TRKPT) {
            strings::to_double(value, _temp.m_longitude);
        }
    }

    void GpxParser::CharData(string data) {
        if (_tags.size() < 2)
            return;

        strings::Trim(data);
        size_t n = _tags.size();
        string curTag = _tags[n - 1];
        string prevTag = _tags[n - 2];

        if (prevTag == GPX_TRKPT) {
            if (curTag == GPX_TIME) {
                _temp.m_timestamp = my::StringToTimestamp(data);

            } else if (curTag == GPX_ELE) {
                strings::to_double(data, _temp.m_altitude);

            } else if (curTag == GPX_SPEED) {
                strings::to_double(data, _temp.m_speed);
            }
        }

    }

    string const & GpxParser::getTagFromEnd(size_t i) {
        if ( i >= _tags.size())
            return "";

        return _tags[_tags.size() - i - 1];
    }

}

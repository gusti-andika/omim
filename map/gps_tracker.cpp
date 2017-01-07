#include "map/gps_tracker.hpp"
#include "map/framework.hpp"
#include "map/bookmark.hpp"

#include "drape_frontend/drape_engine.hpp"
#include "coding/file_name_utils.hpp"

#include "platform/platform.hpp"

#include "std/atomic.hpp"
#include "std/chrono.hpp"

#include "defines.hpp"
#include "base/logging.hpp"

#include "geometry/distance_on_sphere.hpp"

#define SET_STATE(state)        \
    m_prevState = m_state;      \
    m_state = state;

namespace
{

char const kEnabledKey[] = "GpsTrackingEnabled";

inline string GetFilePath()
{
  string fileName = my::TimestampToString(time(NULL));
  fileName += ".png";
  fileName.erase(remove(fileName.begin(), fileName.end(), ':'), fileName.end());
  fileName.erase(remove(fileName.begin(), fileName.end(), '-'), fileName.end());
  return my::JoinFoldersToPath(GetPlatform().TmpDir(), fileName);
}

inline bool GetSettingsIsEnabled()
{
  bool enabled = false;
  settings::Get(kEnabledKey, enabled);
  return enabled;
}

inline void SetSettingsIsEnabled(bool enabled)
{
  settings::Set(kEnabledKey, enabled);
}

} // namespace

GpsTracker::GpsTracker(Framework &framework)
  : UserMarkContainer(0.0, UserMarkType::BOOKMARK_MARK,  framework)
  , m_enabled(GetSettingsIsEnabled())
  , m_listener(nullptr)
  , m_hasStartPoint(false)
{
}

GpsTracker::~GpsTracker()
{
}

void GpsTracker::SetEnabled(bool enabled)
{  
  if (enabled == m_enabled)
    return;

  SetSettingsIsEnabled(enabled);
  m_enabled = enabled;
}

bool GpsTracker::IsEnabled() const
{
  return m_enabled;
}

void GpsTracker::SetOnTrackerInfo(const TOnTrackerInfo &fn)
{
    m_OnTrackerInfo = fn;
}

void GpsTracker::OnLocationUpdated(location::GpsInfo const & info)
{
  lock_guard<mutex> guard(m_mutex);
  if (!m_enabled)
    return;

  if (!IsStarted())
      return;

  if (m_prevState == STATE_PAUSED) {
      LOG(LDEBUG, ("SetPausePointMark:", info));
      SetPausePointMark(info);
      m_prevState = STATE_EMPTY;
  }

  if (m_hasStartPoint) {
      CalcTrackerInfo(info);
  }

  if (!m_hasStartPoint) {
    SetStartPointMark(info);
    m_hasStartPoint = true;
  }

  m_lastPoint = info;

  vector<df::GpsTrackPoint> pointsAdd;
  vector<uint32_t> indicesRemove;
  {
    df::GpsTrackPoint pt;
    pt.m_id = 0;
    pt.m_speedMPS = info.m_speed;
    pt.m_timestamp = info.m_timestamp;
    pt.m_point = MercatorBounds::FromLatLon(info.m_latitude, info.m_longitude);
    pointsAdd.emplace_back(pt);
  }

  m_engine->UpdateGpsTrackPoints(move(pointsAdd), move(indicesRemove));

}

void GpsTracker::Load(string const & trackFile)
{
    lock_guard<mutex> guard(m_mutex);
    SET_STATE(STATE_EMPTY);
    m_engine->ClearGpsTrackPoints();
}

void GpsTracker::Start()
{
    lock_guard<mutex> guard(m_mutex);
    if(IsStarted())
        return;

    if (IsPaused()) {
        SET_STATE(STATE_STARTED);
        return;
    }

    SET_STATE(STATE_STARTED);

    if (m_listener)
        m_listener->OnTrackingStarted();
}

void GpsTracker::Stop()
{
    lock_guard<mutex> guard(m_mutex);
    if (IsStopped())
        return;

    m_speeds.clear();
    m_speeds.shrink_to_fit();

    SetEndPointMark(m_lastPoint);

    m_engine->LoseLocation();
    m_engine->ShowGpsTrackPointsRect();

    SET_STATE(STATE_STOPPED);

    m_hasStartPoint = false;
    if (m_listener)
        m_listener->OnTrackingStopped();
}

void GpsTracker::Cancel()
{
    lock_guard<mutex> guard(m_mutex);
    if (IsStopped())
        return;

    ClearMarks();
    m_engine->ClearGpsTrackPoints();

    m_speeds.clear();
    m_speeds.shrink_to_fit();

    if (m_listener)
        m_listener->OnTrackingStopped(true);

    SET_STATE(STATE_STOPPED);

    m_hasStartPoint = false;
}

void GpsTracker::Pause()
{
    lock_guard<mutex> guard(m_mutex);
    if (!IsStarted())
        return;

    SET_STATE(STATE_PAUSED);
    m_engine->PauseGpsTrackPoints();
    SetPausePointMark(m_lastPoint);
}

bool GpsTracker::IsStarted()
{
    return m_state == STATE_STARTED;
}

bool GpsTracker::IsStopped()
{
    return m_state == STATE_STOPPED;
}

bool GpsTracker::IsPaused()
{
    return m_state == STATE_PAUSED;
}

GpsTracker::ETrackerState GpsTracker::GetState()
{
    return m_state.load();
}

void GpsTracker::SetStartPointMark(const location::GpsInfo &info)
{
    Guard guard(*this);
    m2::PointD point = MercatorBounds::FromLatLon(info.m_latitude, info.m_longitude);
    guard.m_controller.SetIsVisible(true);
    guard.m_controller.SetIsDrawable(true);
    static_cast<Bookmark *>(CreateUserMark(point))->SetData(BookmarkData("","route_from"));
}

void GpsTracker::SetPausePointMark(const location::GpsInfo &info)
{
    Guard guard(*this);
    m2::PointD point = MercatorBounds::FromLatLon(info.m_latitude, info.m_longitude);
    guard.m_controller.SetIsVisible(true);
    guard.m_controller.SetIsDrawable(true);
    static_cast<Bookmark *>(CreateUserMark(point))->SetData(BookmarkData("","route_pause"));
}

void GpsTracker::SetEndPointMark(const location::GpsInfo &info)
{
    Guard guard(*this);
    m2::PointD point = MercatorBounds::FromLatLon(info.m_latitude, info.m_longitude);
    static_cast<Bookmark *>(CreateUserMark(point))->SetData(BookmarkData("","route_to"));
}

void GpsTracker::ClearMarks()
{
    Guard guard(*this);
    guard.m_controller.Clear();
    m_engine->ClearGpsTrackPoints();
}

UserMark * GpsTracker::AllocateUserMark(m2::PointD const & ptOrg)
{
  return new Bookmark(ptOrg, this);
}

void GpsTracker::CalcTrackerInfo(location::GpsInfo const &info)
{
    m_trackerInfo.distance += ms::DistanceOnEarth(info.m_latitude, info.m_longitude,
                                                  m_lastPoint.m_latitude, m_lastPoint.m_longitude);
    m_trackerInfo.lastSpeed = info.m_speed;

    m_speeds.push_back(info.m_speed);
    if (m_speeds.size() > 1) {
        size_t size = m_speeds.size();
        double sum = 0;
        std::for_each(m_speeds.begin(), m_speeds.end(),
                      [&](double const &s){
            sum+=s;
        });

        m_trackerInfo.avgSpeed = sum/size;
    }

    if (m_OnTrackerInfo)
        m_OnTrackerInfo(m_trackerInfo);
}

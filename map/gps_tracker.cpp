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

#include <ctime>
namespace
{

char const kEnabledKey[] = "GpsTrackingEnabled";
char const kDurationHours[] = "GpsTrackingDuration";
uint32_t constexpr kDefaultDurationHours = 24;

size_t constexpr kMaxItemCount = 100000; // > 24h with 1point/s

inline string GetFilePath()
{
  string fileName = my::TimestampToString(time(NULL)) + ".dat";
  return my::JoinFoldersToPath(GetPlatform().WritableDir(), fileName);
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

inline hours GetSettingsDuration()
{
  uint32_t duration = kDefaultDurationHours;
  settings::Get(kDurationHours, duration);
  return hours(duration);
}

inline void SetSettingsDuration(hours duration)
{
  uint32_t const hours = duration.count();
  settings::Set(kDurationHours, hours);
}

} // namespace

GpsTracker::GpsTracker(Framework &framework)
  : UserMarkContainer(0.0, UserMarkType::BOOKMARK_MARK,  framework)
  , m_enabled(GetSettingsIsEnabled())
  , m_listener(nullptr)
  , m_started(false)
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

  if (enabled && m_track)
    m_track->Clear();
}

bool GpsTracker::IsEnabled() const
{
  return m_enabled;
}

void GpsTracker::SetDuration(hours duration)
{
  SetSettingsDuration(duration);
  if(m_track)
    m_track->SetDuration(duration);
}

hours GpsTracker::GetDuration() const
{
  return m_track ? m_track->GetDuration() : hours(0) ;
}

void GpsTracker::Connect(TGpsTrackDiffCallback const & fn)
{
  m_trackDiffCallback = fn;
  if (m_track)
    m_track->SetCallback(fn);
}

void GpsTracker::Disconnect()
{
  if(m_track)
    m_track->SetCallback(nullptr);
}

void GpsTracker::OnLocationUpdated(location::GpsInfo const & info)
{
  lock_guard<mutex> guard(m_mutex);
  if (!m_enabled || !m_track || !m_started)
    return;

  m_track->AddPoint(info);
  if (!m_hasStartPoint) {
    SetStartPointMark(info);
    m_hasStartPoint = true;
  }

  m_lastPoint = info;
}

void GpsTracker::Load(string const & trackFile)
{
    lock_guard<mutex> guard(m_mutex);
    m_started = false;
    m_engine->ClearGpsTrackPoints();
    m_track.reset(new GpsTrack(trackFile,
                               kMaxItemCount,
                               GetSettingsDuration(),
                               make_unique<GpsTrackNullFilter>()));

    m_track->SetCallback(m_trackDiffCallback);
}

void GpsTracker::Start()
{
    lock_guard<mutex> guard(m_mutex);
    if(m_started)
        return;
    m_track.reset(new GpsTrack(GetFilePath(),
                               kMaxItemCount,
                               GetSettingsDuration(),
                               make_unique<GpsTrackNullFilter>()));
    m_track->SetCallback(m_trackDiffCallback);
    m_started = true;
    if (m_listener)
        m_listener->OnTrackingStarted();
}

void GpsTracker::Stop()
{
    lock_guard<mutex> guard(m_mutex);
    if (!m_started || !m_track)
        return;

    m_track->Save();
    m_track.reset();

    SetEndPointMark(m_lastPoint);

    m_engine->LoseLocation();
    m_engine->ShowGpsTrackPointsRect();

    m_started = false;
    if (m_listener)
        m_listener->OnTrackingStopped();
}

void GpsTracker::Cancel()
{
    lock_guard<mutex> guard(m_mutex);
    if (!m_started)
        return;

    ClearMarks();
    m_engine->ClearGpsTrackPoints();

    m_track.reset();

    if (m_listener)
        m_listener->OnTrackingStopped(true);
    m_started = false;
}

bool GpsTracker::IsStarted()
{
    return m_started;
}

void GpsTracker::SetStartPointMark(const location::GpsInfo &info)
{
    Guard guard(*this);
    m2::PointD point = MercatorBounds::FromLatLon(info.m_latitude, info.m_longitude);
    guard.m_controller.SetIsVisible(true);
    guard.m_controller.SetIsDrawable(true);
    static_cast<Bookmark *>(CreateUserMark(point))->SetData(BookmarkData("","route_from"));
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
}


UserMark * GpsTracker::AllocateUserMark(m2::PointD const & ptOrg)
{
  return new Bookmark(ptOrg, this);
}




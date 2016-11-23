#include "map/gps_tracker.hpp"
#include "map/framework.hpp"
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

GpsTracker & GpsTracker::Instance()
{
  static GpsTracker instance;
  return instance;
}

GpsTracker::GpsTracker()
  : m_enabled(GetSettingsIsEnabled())
  , m_started(false), m_listener(nullptr)
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
  if (!m_enabled || !m_track)
    return;
  m_track->AddPoint(info);
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
    m_engine->LoseLocation();
    m_engine->ShowGpsTrackPointsRect();
    if (m_listener)
        m_listener->OnTrackingStopped();
    m_started = false;
}

void GpsTracker::Cancel()
{
    lock_guard<mutex> guard(m_mutex);
    if (!m_started)
        return;

    m_track.reset();
    m_engine->ClearGpsTrackPoints();
    if (m_listener)
        m_listener->OnTrackingStopped(true);
    m_started = false;
}

bool GpsTracker::IsStarted()
{
    return m_started;
}



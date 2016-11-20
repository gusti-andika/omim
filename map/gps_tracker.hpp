#pragma once

#include "map/gps_track.hpp"

#include "std/atomic.hpp"

#include "std/unique_ptr.hpp"

class Framework;
class GpsTracker
{
public:
  static GpsTracker & Instance();

  bool IsEnabled() const;
  void SetEnabled(bool enabled);

  hours GetDuration() const;
  void SetDuration(hours duration);

  using TGpsTrackDiffCallback = std::function<void(vector<pair<size_t, location::GpsTrackInfo>> && toAdd,
                                                   pair<size_t, size_t> const & toRemove)>;

  void Connect(TGpsTrackDiffCallback const & fn);
  void Disconnect();
  void Load(string const & trackFile);
  void Start();
  void Stop();
  void Cancel();
  void OnLocationUpdated(location::GpsInfo const & info);
  bool IsStarted();
  void SetFramework(Framework &f);

private:
  GpsTracker();
  TGpsTrackDiffCallback m_trackDiffCallback;
  atomic<bool> m_enabled;
  unique_ptr<GpsTrack> m_track;
  Framework *m_framework;
  bool m_started;
};

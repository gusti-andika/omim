#pragma once

#include "map/gps_track.hpp"

#include "std/atomic.hpp"
#include "std/mutex.hpp"

#include "drape/pointers.hpp"
#include "std/unique_ptr.hpp"

namespace df{
class DrapeEngine;
}

class Framework;
class GpsTracker
{
public:

    class ActivationListener
    {
    public:
        virtual void OnTrackingStarted() = 0;
        virtual void OnTrackingStopped(bool cancelled=false) = 0;
    };

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
  void SetEngine(ref_ptr<df::DrapeEngine> drape) {
      m_engine = drape;
  }

  void SetListener(ActivationListener *listener) {
      m_listener = listener;
  }

private:
  GpsTracker();
  TGpsTrackDiffCallback m_trackDiffCallback;
  atomic<bool> m_enabled;
  unique_ptr<GpsTrack> m_track;
  ref_ptr<df::DrapeEngine> m_engine;
  ActivationListener *m_listener;
  bool m_started;
  mutex m_mutex;
};

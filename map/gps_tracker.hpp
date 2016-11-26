#pragma once

#include "map/gps_track.hpp"
#include "map/user_mark_container.hpp"

#include "std/atomic.hpp"
#include "std/mutex.hpp"
#include "std/unique_ptr.hpp"

#include "drape/pointers.hpp"

#include "platform/location.hpp"

namespace df{
class DrapeEngine;
}

class Framework;

class GpsTracker : public UserMarkContainer
{
public:

    class ActivationListener
    {
    public:
        virtual void OnTrackingStarted() = 0;
        virtual void OnTrackingStopped(bool cancelled=false) = 0;
    };

    class Guard {
    public :
        Guard(GpsTracker &tracker)
            : m_controller(tracker.RequestController())
            , m_tracker(tracker){

        }

        ~Guard()
        {
            m_tracker.ReleaseController();
        }

        UserMarksController &m_controller;
    private:

        GpsTracker &m_tracker;
    };

  //static GpsTracker & Instance();

  GpsTracker(Framework &framework);
  ~GpsTracker();

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
  void SetStartPointMark(location::GpsInfo const &info);
  void SetEndPointMark(location::GpsInfo const &info);
  void ClearMarks();

protected:
  UserMark * AllocateUserMark(m2::PointD const & ptOrg) override;

private:
  DISALLOW_COPY_AND_MOVE(GpsTracker);

  TGpsTrackDiffCallback m_trackDiffCallback;
  atomic<bool> m_enabled;
  unique_ptr<GpsTrack> m_track;
  ref_ptr<df::DrapeEngine> m_engine;
  ActivationListener *m_listener = nullptr;
  atomic<bool> m_started;
  mutex m_mutex;

  bool m_hasStartPoint;
  location::GpsInfo m_lastPoint;
};

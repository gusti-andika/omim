#pragma once

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

    struct TrackerInfo{
        double avgSpeed;
        double lastSpeed;
        double distance;
        string time;

        TrackerInfo() = default;
        TrackerInfo(TrackerInfo const &) = default;
        TrackerInfo & operator=(TrackerInfo const &) = default;

    };

    enum ETrackerState {
        STATE_EMPTY = 0,
        STATE_STARTED,
        STATE_PAUSED,
        STATE_STOPPED
    } ;

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

  GpsTracker(Framework &framework);
  ~GpsTracker();

  bool IsEnabled() const;
  void SetEnabled(bool enabled);

  using TOnTrackerInfo = std::function<void(const TrackerInfo &info)>;

  void SetOnTrackerInfo(TOnTrackerInfo const &fn);

  void Load(string const & trackFile);

  void Start();
  void Stop();
  void Pause();
  void Cancel();
  ETrackerState GetState();

  void OnLocationUpdated(location::GpsInfo const & info);

  bool IsStarted();

  bool IsPaused();

  bool IsStopped();

  void SetEngine(ref_ptr<df::DrapeEngine> drape) {
      m_engine = drape;
  }

  void SetListener(ActivationListener *listener) {
      m_listener = listener;
  }
  void SetStartPointMark(location::GpsInfo const &info);
  void SetEndPointMark(location::GpsInfo const &info);
  void SetPausePointMark(location::GpsInfo const &info);

  void ClearMarks();

protected:
  UserMark * AllocateUserMark(m2::PointD const & ptOrg) override;

private:
  DISALLOW_COPY_AND_MOVE(GpsTracker);

  void CalcTrackerInfo(location::GpsInfo const &info);

  TOnTrackerInfo m_OnTrackerInfo;
  atomic<bool> m_enabled;

  ref_ptr<df::DrapeEngine> m_engine;
  ActivationListener *m_listener = nullptr;
  atomic<ETrackerState> m_state{STATE_EMPTY};
  ETrackerState m_prevState{STATE_EMPTY};
  mutex m_mutex;

  bool m_hasStartPoint;
  location::GpsInfo m_lastPoint;
  TrackerInfo m_trackerInfo = {0,0,0,""};
  vector<double> m_speeds;
};

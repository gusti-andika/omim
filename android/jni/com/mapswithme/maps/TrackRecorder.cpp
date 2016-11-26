#include "Framework.hpp"

#include "map/gps_tracker.hpp"

#include "std/chrono.hpp"

namespace
{

::Framework * frm()
{
  return (g_framework ? g_framework->NativeFramework() : nullptr);
}

}  // namespace

extern "C"
{
  JNIEXPORT void JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeSetEnabled(JNIEnv * env, jclass clazz, jboolean enable)
  {
    //GpsTracker::Instance().SetEnabled(enable);
    Framework * const f = frm();
    if (f == nullptr)
      return;

    f->GetGpsTracker().SetEnabled(enable);
    
    if (enable)
      f->ConnectToGpsTracker();
    else
      f->DisconnectFromGpsTracker();
  }

  JNIEXPORT jboolean JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeIsEnabled(JNIEnv * env, jclass clazz)
  {
    Framework * const f = frm();
    if (f == nullptr)
      return false;
    return f->GetGpsTracker().IsEnabled();
  }

  JNIEXPORT void JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeSetDuration(JNIEnv * env, jclass clazz, jint durationHours)
  {
    Framework * const f = frm();
    if (f != nullptr)
    	f->GetGpsTracker().SetDuration(hours(durationHours));
  }

  JNIEXPORT jint JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeGetDuration(JNIEnv * env, jclass clazz)
  {
    Framework * const f = frm();
    if (f == nullptr)
	return -1;
    return f->GetGpsTracker().GetDuration().count();
  }
}

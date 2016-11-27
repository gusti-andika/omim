#include "Framework.hpp"
#include "com/mapswithme/core/jni_helper.hpp"
#include "std/unique_ptr.hpp"
#include "map/gpx_playback.hpp"

namespace
{
::Framework * frm()
{
  return g_framework->NativeFramework();
}

unique_ptr<gpx::GpxPlayback> gpxPlayback;
}



extern "C"
{
JNIEXPORT void JNICALL
Java_com_mapswithme_maps_GpsTracker_startGpxPlayback(JNIEnv * env, jclass clazz, jstring file)
{
    string str = jni::ToNativeString(env, file);
    gpxPlayback.reset(new gpx::GpxPlayback(str));
    gpxPlayback->SetOnLocationUpdateCallback(std::bind(&android::Framework::OnLocationUpdated, g_framework, _1));
    gpxPlayback->Play();
}

JNIEXPORT void JNICALL
Java_com_mapswithme_maps_GpsTracker_stopGpxPlayback(JNIEnv * env, jclass clazz)
{
    if (!gpxPlayback)
        return;

    gpxPlayback->Stop();
}

JNIEXPORT void JNICALL
Java_com_mapswithme_maps_GpsTracker_startTracking(JNIEnv * env, jclass clazz)
{
    frm()->GetGpsTracker().Start();
}

JNIEXPORT void JNICALL
Java_com_mapswithme_maps_GpsTracker_stopTracking(JNIEnv * env, jclass clazz, jboolean cancel)
{
    if (!cancel)
        frm()->GetGpsTracker().Stop();
    else
        frm()->GetGpsTracker().Cancel();
}

JNIEXPORT void JNICALL
Java_com_mapswithme_maps_GpsTracker_clearTracking(JNIEnv * env, jclass clazz)
{
    frm()->GetGpsTracker().ClearMarks();
}


}

#include "Framework.hpp"
#include "../core/jni_helper.hpp"
#include "../core/ScopedEnv.hpp"
#include "std/unique_ptr.hpp"
#include "std/shared_ptr.hpp"
#include "std/bind.hpp"
#include "map/gps_tracker.hpp"
#include "map/gpx_playback.hpp"
#include "base/logging.hpp"

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

static jobject CreateTrackerInfoObject(JNIEnv *env, GpsTracker::TrackerInfo const &info)
{
    static jmethodID const trackerInfoConstructor = jni::GetConstructorID(env, g_trackerInfoClazz,
                    "(DDD)V");

    return env->NewObject(g_trackerInfoClazz, trackerInfoConstructor,
                                        info.avgSpeed, info.lastSpeed, info.distance);
}

static void OnTrackerInfoReceived(shared_ptr<jobject> const &listener, GpsTracker::TrackerInfo const &info)
{
    ScopedEnv env(jni::GetJVM());

    jmethodID const method = jni::GetMethodID(env.get(), *listener, "onTrackerInfoUpdated",
            "(Lcom/mapswithme/maps/tracker/TrackerController$TrackerInfo;)V");
    ASSERT(method, ());
    jni::TScopedLocalRef infoObject(env.get(), CreateTrackerInfoObject(env.get(), info));

    env->CallVoidMethod(*listener, method, infoObject.get());

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

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
Java_com_mapswithme_maps_GpsTracker_pauseTracking(JNIEnv * env, jclass clazz)
{
    frm()->GetGpsTracker().Pause();
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

JNIEXPORT void JNICALL
Java_com_mapswithme_maps_GpsTracker_setTrackerInfoListener(JNIEnv * env, jclass clazz, jobject listener)
{
    frm()->GetGpsTracker().SetOnTrackerInfo(bind(&OnTrackerInfoReceived, jni::make_global_ref(listener), _1));
}

JNIEXPORT jobject JNICALL
Java_com_mapswithme_maps_GpsTracker_getState(JNIEnv * env, jclass clazz)
{
    static jclass gpsTrackerStateClass = jni::GetGlobalClassRef(env, "com/mapswithme/maps/GpsTracker$State");
    static jfieldID fStateEmpty = jni::GetStaticFieldID(env, gpsTrackerStateClass, "Empty", "com/mapswithme/maps/GpsTracker$State");
    static jfieldID fStateStarted = jni::GetStaticFieldID(env, gpsTrackerStateClass, "Started", "com/mapswithme/maps/GpsTracker$State");
    static jfieldID fStatePaused = jni::GetStaticFieldID(env, gpsTrackerStateClass, "Paused", "com/mapswithme/maps/GpsTracker$State");
    static jfieldID fStateStopped = jni::GetStaticFieldID(env, gpsTrackerStateClass, "Stopped", "com/mapswithme/maps/GpsTracker$State");

    GpsTracker::ETrackerState nativeState = frm()->GetGpsTracker().GetState();

    switch(nativeState) {
        case GpsTracker::STATE_STARTED:
            return env->GetStaticObjectField(gpsTrackerStateClass, fStateStarted);
        case GpsTracker::STATE_PAUSED:
            return env->GetStaticObjectField(gpsTrackerStateClass, fStatePaused);
        case GpsTracker::STATE_STOPPED:
            return env->GetStaticObjectField(gpsTrackerStateClass, fStateStopped);
        default:
            return env->GetStaticObjectField(gpsTrackerStateClass, fStateEmpty);
    }
}

}

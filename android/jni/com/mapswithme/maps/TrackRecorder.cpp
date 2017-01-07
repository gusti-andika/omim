#include <jni.h>

extern "C"
{
  JNIEXPORT void JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeSetEnabled(JNIEnv * env, jclass clazz, jboolean enable)
  {

  }

  JNIEXPORT jboolean JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeIsEnabled(JNIEnv * env, jclass clazz)
  {
    return false;
  }

  JNIEXPORT void JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeSetDuration(JNIEnv * env, jclass clazz, jint durationHours)
  {

  }

  JNIEXPORT jint JNICALL
  Java_com_mapswithme_maps_location_TrackRecorder_nativeGetDuration(JNIEnv * env, jclass clazz)
  {
        return 0;
  }
}

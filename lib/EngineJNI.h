/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class EngineJNI */

#ifndef _Included_EngineJNI
#define _Included_EngineJNI
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     EngineJNI
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_EngineJNI_initialize
    (JNIEnv *, jclass);

/*
 * Class:     EngineJNI
 * Method:    finish
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_EngineJNI_finish
    (JNIEnv *, jclass);

/*
 * Class:     EngineJNI
 * Method:    execute
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_EngineJNI_execute
    (JNIEnv *, jclass, jstring);

/*
 * Class:     EngineJNI
 * Method:    getCurrentDatabaseName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_EngineJNI_getCurrentDatabaseName
    (JNIEnv *env, jclass);

#ifdef __cplusplus
}
#endif
#endif

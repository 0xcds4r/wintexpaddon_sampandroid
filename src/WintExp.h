#ifndef _H_WINTEXP_
#define _H_WINTEXP_

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <list>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <jni.h>
#include <ucontext.h>
#include <android/log.h>

#define GTASA_LIBRARY_NAME "libGTASA.so"
#define GAME_STATE_ADDR 0xA987C8

#ifndef DEBUG_LOG
	#define LOG(...) ((void)0)
#else
	#define LOG_TAG "WintExp-Debug"
	#ifdef LOG_TAG
		#define LOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);
	#endif
#endif

#endif

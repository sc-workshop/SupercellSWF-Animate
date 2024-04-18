/*************************************************************************
* ADOBE CONFIDENTIAL
* ___________________
*
*  Copyright [2013] Adobe Systems Incorporated
*  All Rights Reserved.
*
* NOTICE:  All information contained herein is, and remains
* the property of Adobe Systems Incorporated and its suppliers,
* if any.  The intellectual and technical concepts contained
* herein are proprietary to Adobe Systems Incorporated and its
* suppliers and are protected by all applicable intellectual
* property laws, including trade secret and copyright laws.
* Dissemination of this information or reproduction of this material
* is strictly forbidden unless prior written permission is obtained
* from Adobe Systems Incorporated.
**************************************************************************/

// Must be balanced with FCMPostConfig.h!!!!!

#ifdef __FCM__CONFIG__
	#error "FCMPostConfig.h is not included in last file"
#endif

#ifndef __FCM__
#define __FCM__
#endif // !__FCM__

#if defined(__x86_64__) || defined(_M_X64)
	#define FCM_OS_64
#elif defined(__ppc64__)
	#define FCM_OS_64
#else
	#error "Unsupported FCM architecture"
#endif

#if defined(_WIN64)
#define FCM_OS_WIN
#elif defined(__MWERKS__) || defined(__APPLE__)
#define FCM_OS_MAC
#else
#error "Unsupported platform"
#endif

#if defined(FCM_OS_64)
	#define FCM_ALIGNMENT 16
#else
	#error "Unsupported platform bit depth"
#endif

#ifdef FCM_OS_WIN
	#pragma warning(push)

	/**   Disabled Warnings **/

	// An include file uses the pack pragma to change the default structure alignment.
	#pragma warning( disable : 4103)

	// Most of classes anyway does not have accessible constructor or destructor
	// So we dont need virtual destructors
	#pragma warning( disable : 5204)
	
	// We know for sure that pop will be called and we can provide a guarantee at compiler level
	#pragma warning( disable : 5031)

	#include "win/FCMPreAlign_WIN.h"

#elif defined(FCM_OS_MAC)

	#include "mac/FCMPreAlign_MAC.h"

#else
	#error "Unsupported platform"
#endif

#define __FCM__CONFIG__
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define ESP_PLATFORM        1
#define LEWEI_PLATFORM      0

#if ESP_PLATFORM
#define PLUG_DEVICE         1
#define LIGHT_DEVICE        0
#define HUMITURE_DEVICE     0

#ifdef LIGHT_DEVICE
#define USE_US_TIMER
#endif

#if PLUG_DEVICE || LIGHT_DEVICE
#define BEACON_TIMEOUT  150000000
#define BEACON_TIME     50000
#endif

#elif LEWEI_PLATFORM
#endif

#endif


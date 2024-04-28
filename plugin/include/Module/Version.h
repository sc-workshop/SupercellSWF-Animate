#pragma once

#include "PluginConfiguration.h"

#define PLUGIN_VERSION ( (PLUGIN_VERSION_MAJOR << 24) | (PLUGIN_VERSION_MINOR << 16) | (PLUGIN_VERSION_MAINTENANCE << 8) )

#define SC_STR_HELPER(str) #str
#define SC_STR(str) SC_STR_HELPER(str)

const char PluginVersion[] = 
	"Version: " SC_STR(PLUGIN_VERSION_MAJOR) "." SC_STR(PLUGIN_VERSION_MINOR) "." SC_STR(PLUGIN_VERSION_MAINTENANCE) " | "
	"Build date: " __DATE__ " " __TIME__
;

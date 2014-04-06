#pragma once

#include "stdinc.h"
#include "PluginController.h"

namespace archStatusLog
{
	class CntCache
	{
		vector<ProtocolInfo> _cnts;
	public:
		CntCache()
		{
			WTWFUNCTIONS *wtw = PluginController::getInstance().getWTWFUNCTIONS();
			
			wtw->fnCall(WTW_SETTINGS_GET_MY_CONFIG_FILE, reinterpret_cast<WTW_PARAM>(&configName), reinterpret_cast<WTW_PARAM>(hInst));
		}
	};
};

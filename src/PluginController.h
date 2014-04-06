#pragma once

#include "stdinc.h"
#include "ProtocolInfo.h"
#include "ProtoIcons.h"
#include "ArchPage.h"

namespace archStatusLog {

class Dtb; // forward declaration

/** Singleton */
class PluginController
{
	// basic
	WTWFUNCTIONS*	_wtw;
	// hooks
	void*			_statusHook;
	void*			_allPluginsLoadedHook;
	//void*			_protocolAddHook;
	// protocols info
	std::vector<ProtocolInfo>		_protosInfos;
	ProtoIcons*		_protoIcons;
	ArchPage*		_archPage;

	PluginController() : _wtw(NULL), hInst(NULL), _statusHook(NULL),
		_allPluginsLoadedHook(NULL), _protoIcons(NULL), dtb(NULL) {}
	PluginController(const PluginController&);

	// callbacks
	static WTW_PTR onStatusChange(WTW_PARAM wParam, WTW_PARAM lParam, void *cbData);
	static WTW_PTR enumProtocols(WTW_PARAM wParam, WTW_PARAM lParam, void *cbData);
	static WTW_PTR archiveCallback(WTW_PARAM wParam, WTW_PARAM lParam, void *cbData);
public:
	Dtb*			dtb;
	HINSTANCE		hInst;

	static PluginController& getInstance()
	{
		static PluginController instance;
		return instance;
	}

	int onLoad(WTWFUNCTIONS *wtw);
	int onUnload();
	inline const WTWPLUGINFO* getPlugInfo()
	{
		static WTWPLUGINFO _plugInfo = {
			sizeof(WTWPLUGINFO),						// struct size
			L"archStatusLog",							// plugin name
			L"Zapisywanie zmian statusu i opisu do archiwum", // plugin description
			L"© 2010-2014 Maciej Muszkowski",			// copyright
			L"Maciej Muszkowski",						// author
			L"maciek.muszkowski@gmail.com",				// authors contact
			L"http://www.alset.pl/Maciek",				// authors webpage
			L"",										// url to xml with autoupdate data
			PLUGIN_API_VERSION,							// api version
			MAKE_QWORD(2, 2, 0, 0),						// plugin version
			WTW_CLASS_ARCHIVE,							// plugin class
			NULL,										// function called after "O wtyczce..." pressed
			L"{5afcaa91-ab8f-4c6b-9bfb-1443ee220569}",	// guid
			NULL,										// dependencies (list of guids)
			WTW_PLUGIN_OPTION_NO_MANUAL_LOAD,			// options	
			0, 0, 0										// reserved
		};
		return &_plugInfo;
	}

	inline WTWFUNCTIONS* getWTWFUNCTIONS() const
	{
		return _wtw;
	}
};

}; // namespace archStatusLog

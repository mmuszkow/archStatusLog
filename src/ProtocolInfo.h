#pragma once

#include "stdinc.h"

using strUtils::utfstring;

namespace archStatusLog {

class ProtocolInfo
{
public:
	/** Name of net */
	utfstring	net;
	/** Id of account of net (we can have more than 1 accout of net) */
	int			sid;
	/** Contact id (UID) */
	utfstring	uid;
	/** This will be displayed on tree */
	utfstring	display;
	/** Standard ("") or special icon name format */
	utfstring	icon;

	void assign(ProtocolInfo const&pi);

	ProtocolInfo() : sid(0) {}
	ProtocolInfo(const ProtocolInfo& pi2) : sid(0) { assign(pi2); }
	ProtocolInfo(const wchar_t* _net, const int _sid, const wchar_t* _uid, 
		const wchar_t* _display, const wchar_t* _icon);

	ProtocolInfo& operator=(const ProtocolInfo &pi2) { assign(pi2); return *this; }

	bool operator==(const ProtocolInfo &pi2) { return (net == pi2.net && sid==pi2.sid); }
};

}; // namespace archStatusLog

#include "stdinc.h"
#include "ProtocolInfo.h"


namespace archStatusLog
{
	ProtocolInfo::ProtocolInfo(const wchar_t* _net, const int _sid, 
		const wchar_t* _uid, const wchar_t* _display, const wchar_t* _icon)
		: sid(0)
	{
		if(_net) net.assign(_net);
		sid = _sid;
		if(_uid) uid.assign(_uid);
		if(_icon) icon.assign(_icon);
		if(_display) display.assign(_display);
	}

	void ProtocolInfo::assign(ProtocolInfo const&pi2)
	{
		net.assign(pi2.net);
		sid = pi2.sid;
		uid.assign(pi2.uid);
		icon.assign(pi2.icon);
		display.assign(pi2.display);
	}
};
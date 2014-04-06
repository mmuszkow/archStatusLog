#pragma once

#include "stdinc.h"
#include "ProtocolInfo.h"

using strUtils::utfstring;

namespace archStatusLog
{
	/** Class that load and holds the protocols icons for list & tree view */
	class ProtoIcons
	{
		std::vector<ProtocolInfo>*	pInfos;
		/** Number of protocols before creation of ProtocolsIconsLoader,
			if any new protocol is added to protosInfos later, it's icons 
			won't be loaded and for each status err icon will be returned
			*/
		int							protosCountCrtn;
		HIMAGELIST					hTreeImgList;
		ProtoIcons() : hTreeImgList(NULL), pInfos(NULL), protosCountCrtn(0) {}
	public:
		static const int MAX_STATUS = WTW_PRESENCE_OFFLINE-WTW_PRESENCE_ONLINE+1;
		ProtoIcons(WTWFUNCTIONS *wtw, std::vector<ProtocolInfo>*);
		~ProtoIcons();

		int getId(utfstring& net, int sid, int status);
		HIMAGELIST getImgLst() { return hTreeImgList; }
		inline int count() { return protosCountCrtn; }
	};
};
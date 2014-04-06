#include "stdinc.h"
#include "ProtoIcons.h"

namespace archStatusLog {

ProtoIcons::ProtoIcons(WTWFUNCTIONS* wtw, std::vector<ProtocolInfo>* _protosInfos) 
: hTreeImgList(NULL), pInfos(_protosInfos), protosCountCrtn(0)
{
	if (!_protosInfos)
		return;

	protosCountCrtn = _protosInfos->size();

	hTreeImgList = ImageList_Create(16, 16, ILC_COLOR32, protosCountCrtn*MAX_STATUS+1, 0); // +1 because we need an error icon
	if(!hTreeImgList)
		return;

	wtwGraphics graph;
	graph.flags = WTW_GRAPH_FLAG_GENERATE_HBITMAP;

	HBITMAP hBM = NULL;

	for (int i = 0; i < protosCountCrtn; i++)
	{
		for (int j = WTW_PRESENCE_ONLINE; j < WTW_PRESENCE_OFFLINE+1; j++)
		{
			utfstring iconName; // preparing the struct for getting HBITMAP for each icon
			if(!pInfos->at(i).icon.empty()) // format <protoIconId>/<STATUS_NAME>
			{
				switch (j) 
				{
				case WTW_PRESENCE_ONLINE: 
					iconName.format(L"%s/%s", pInfos->at(i).icon.toUTF16(), L"available");
					break;
				case WTW_PRESENCE_CHAT:
					iconName.format(L"%s/%s", pInfos->at(i).icon.toUTF16(), L"chat");
					break;
				case WTW_PRESENCE_DND:
					iconName.format(L"%s/%s", pInfos->at(i).icon.toUTF16(), L"dnd");
					break;
				case WTW_PRESENCE_AWAY:
					iconName.format(L"%s/%s", pInfos->at(i).icon.toUTF16(), L"away");
					break;
				case WTW_PRESENCE_XA:
					iconName.format(L"%s/%s", pInfos->at(i).icon.toUTF16(), L"xa");
					break;
				case WTW_PRESENCE_INV:
					iconName.format(L"%s/%s", pInfos->at(i).icon.toUTF16(), L"invisible");
					break;
				default: // if not found we put the offline icon
					iconName.format(L"%s/%s", pInfos->at(i).icon.toUTF16(), L"unavailable"); 
				}
			}
			else // format <NET_ID>/Icon/<STATUS_NAME>
			{
				switch (j) 
				{
				case WTW_PRESENCE_ONLINE: 
					iconName.format(L"%s/Icon/%s", pInfos->at(i).net.toUTF16(), L"available");
					break;
				case WTW_PRESENCE_CHAT:
					iconName.format(L"%s/Icon/%s", pInfos->at(i).net.toUTF16(), L"chat");
					break;
				case WTW_PRESENCE_DND:
					iconName.format(L"%s/Icon/%s", pInfos->at(i).net.toUTF16(), L"dnd");
					break;
				case WTW_PRESENCE_AWAY:
					iconName.format(L"%s/Icon/%s", pInfos->at(i).net.toUTF16(), L"away");
					break;
				case WTW_PRESENCE_XA:
					iconName.format(L"%s/Icon/%s", pInfos->at(i).net.toUTF16(), L"xa");
					break;
				case WTW_PRESENCE_INV:
					iconName.format(L"%s/Icon/%s", pInfos->at(i).net.toUTF16(), L"invisible");
					break;
				default: // if not found we put the offline icon
					iconName.format(L"%s/Icon/%s", pInfos->at(i).net.toUTF16(), L"unavailable"); 
				}
			}
			graph.graphId = iconName.toUTF16();
			int id = i * MAX_STATUS + ((j & 0xF) - 1);
			hBM = reinterpret_cast<HBITMAP>(wtw->fnCall(WTW_GRAPH_GET_IMAGE, graph, NULL)); // getting the handle
			if(!hBM) // if not succeded we set the HBITMAP for err icon HBITMAP
			{
				__LOG_F(wtw, WTW_LOG_LEVEL_DEBUG, MDL, L"Icon %s not loaded, replacing with default icon", iconName.toUTF16());
				graph.graphId = WTW_GRAPH_ID_NOL;
				hBM = reinterpret_cast<HBITMAP>(wtw->fnCall(WTW_GRAPH_GET_IMAGE, graph, NULL));
			}
			ImageList_Add(hTreeImgList, hBM, (HBITMAP)NULL);
			//if(hBM) // do not free object, WTW do it itself
			//	DeleteObject(hBM);
		}
	}

	graph.graphId = WTW_GRAPH_ID_NOL; // adding error icon
	hBM = reinterpret_cast<HBITMAP>(wtw->fnCall(WTW_GRAPH_GET_IMAGE, graph, NULL));
	ImageList_Add(hTreeImgList, hBM, (HBITMAP)NULL);

	if(!hBM)
		return;

	__LOG_F(wtw, WTW_LOG_LEVEL_DEBUG, MDL, L"All protocols icons loaded"); // if all succeded
}

ProtoIcons::~ProtoIcons()
{
	if (hTreeImgList)
		ImageList_Destroy(hTreeImgList);
}

int ProtoIcons::getId(utfstring& net, int sid, int status)
{
	if ((status & 0xF) < WTW_PRESENCE_ONLINE || (status & 0xF) > WTW_PRESENCE_OFFLINE)
		return protosCountCrtn*MAX_STATUS; // err icon
	for (int i = 0; i < protosCountCrtn; i++)
		if (pInfos->at(i).net == net && pInfos->at(i).sid == sid)
			return i * MAX_STATUS + ((status & 0xF) - 1);
	return protosCountCrtn*MAX_STATUS; // err icon
}

}; // namespace archStatusLog

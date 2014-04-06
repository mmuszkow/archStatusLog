#include "stdinc.h"
#include "ArchPage.h"
#include "PluginController.h"
#include "Dtb.h"
#include "SqlCmdWnd.h"

namespace archStatusLog {

ArchPage::ArchPage(ProtoIcons* icons, const std::vector<ProtocolInfo>* _protos,
	int x, int y, int cx, int cy, HWND parent) : 
		_hTreeView(NULL), _hListView(NULL), _icons(icons), _parent(parent), 
		last100(false), firstShow(true), lastMonth(true), busy(false)
{
	protos = const_cast<std::vector<ProtocolInfo>*>(_protos);
	if(!protos)
		return;

	PluginController::getInstance().dtb->busy = &busy;

	SetProp(_parent,L"ARCHPAGE",this);

	_hCombo = CreateWindow(WC_COMBOBOX, 0, 
                            CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
							x+MARGIN, y+MARGIN, LIST_W-2, COMBO_H, _parent, 0, 0, NULL);
	SendMessage(_hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Z ostatnich 30 dni"));
	SendMessage(_hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Wszystkie"));
	SendMessage(_hCombo, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), 0);
	SendMessage(_hCombo, CB_SETCURSEL, 0, 0);

	_hTreeView = CreateWindow(WC_TREEVIEW, L"ArchDescCntsTree",
			WS_CHILD | WS_BORDER | WS_TABSTOP | TVS_HASBUTTONS,
			x+MARGIN, 
			y+MARGIN+COMBO_H+MARGIN, 
			LIST_W-2, 
			cy-(MARGIN<<1)-COMBO_H-MARGIN, 
			_parent, 0, 0, NULL);

	_hListView = CreateWindow(WC_LISTVIEW, L"ArchDescList", 
			WS_CHILD | WS_BORDER | WS_TABSTOP | LVS_REPORT, 
			x+MARGIN + LIST_W, 
			y+MARGIN, 
			cx - LIST_W - (MARGIN<<1), 
			cy - (MARGIN<<1), 
			_parent, 0, 0, NULL);
	ListView_SetExtendedListViewStyle(_hListView, LVS_EX_INFOTIP|LVS_EX_LABELTIP|LVS_EX_FULLROWSELECT);
		
	_hSQL = CreateWindowW(L"BUTTON", 0, WS_CHILD, cx-MARGIN, y, MARGIN, MARGIN, _parent, (HMENU)ID_SQLCMD, 0, NULL);

	if(_icons)
	{
		TreeView_SetImageList(_hTreeView, _icons->getImgLst(), TVSIL_NORMAL); // setting HIMAGELIST to tree and list
		ListView_SetImageList(_hListView, _icons->getImgLst(), LVSIL_SMALL);
	}

	SetProp(_parent,L"ARCH_PREVPROC",reinterpret_cast<WNDPROC>(SetWindowLongPtr(_parent, // parent subclass proc
		GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(wndProc))));

	LVCOLUMN lvc; // adding columns to list
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;    
	lvc.cx = 100;
	lvc.pszText = L"Status";
	ListView_InsertColumn (_hListView, 0, &lvc);

	lvc.iSubItem = 3;    
	lvc.cx = 0;
	lvc.pszText = L"Kontakt";
	ListView_InsertColumn (_hListView, 1, &lvc);
	
	lvc.iSubItem = 1;    
	lvc.cx = 200;
	lvc.pszText = L"Opis";
	ListView_InsertColumn (_hListView, 2, &lvc);
	
	lvc.iSubItem = 2;    
	lvc.cx = 150;
	lvc.pszText = L"Data";
	ListView_InsertColumn (_hListView, 3, &lvc);
}

void ArchPage::show()
{
	if(firstShow)
	{
		updateCnts();
		firstShow = false;
	}
	ShowWindow(_hCombo, SW_SHOW);
	ShowWindow(_hTreeView, SW_SHOW);
	ShowWindow(_hListView, SW_SHOW);
	ShowWindow(_hSQL, SW_SHOW);
}

void ArchPage::hide()
{
	ShowWindow(_hCombo, SW_HIDE);
	ShowWindow(_hTreeView, SW_HIDE);
	ShowWindow(_hListView, SW_HIDE);
	ShowWindow(_hSQL, SW_HIDE);
}

void ArchPage::move(int x, int y, int cx, int cy)
{
	MoveWindow(_hCombo, 
		x+MARGIN, 
		y+MARGIN, 
		LIST_W-2, COMBO_H, 
		true);
	MoveWindow(_hTreeView,
		x+MARGIN, 
		y+MARGIN+COMBO_H+MARGIN, 
		LIST_W-2, 
		cy-(MARGIN<<1)-COMBO_H-MARGIN, 
		true);
	MoveWindow(_hListView, 			
		x+MARGIN + LIST_W, 
		y+MARGIN, 
		cx - LIST_W - (MARGIN<<1), 
		cy - (MARGIN<<1),
		true);
	MoveWindow(_hSQL,
		cx-MARGIN,
		y,
		MARGIN,
		MARGIN,
		true);
}


void ArchPage::createWaitWindow()
{
	HWND hWait = CreateDialogW(PluginController::getInstance().hInst, MAKEINTRESOURCE(IDD_WAIT), _parent, NULL);
	SetWindowLong(GetDlgItem(hWait, IDC_MARQUEE), GWL_STYLE, WS_VISIBLE|WS_CHILD|PBS_MARQUEE);
	SendMessage(GetDlgItem(hWait, IDC_MARQUEE), PBM_SETMARQUEE, TRUE, 30);
	ShowWindow(hWait, SW_SHOW);
	PluginController::getInstance().dtb->hBeWait = hWait;
}

void ArchPage::updateCnts()
{
	if(busy)
		return;

	TreeView_DeleteAllItems(_hTreeView);
	ListView_DeleteAllItems(_hListView);

	createWaitWindow();
	if(PluginController::getInstance().dtb)
	{
		std::ostringstream query;
		query <<
			"SELECT DISTINCT wtw_desc.contact_id,wtw_accounts.net,wtw_accounts.login,wtw_desc.account_id " \
			"FROM wtw_desc " \
			"INNER JOIN wtw_accounts ON wtw_desc.account_id=wtw_accounts.account_id";
		if(lastMonth) 
			query << " WHERE wtw_desc.change_date > " << (static_cast<__int64>(time(NULL)) - MONTH) << ";";
		else
			query << ";";

		PluginController::getInstance().dtb->bufferedExecute(query.str().c_str(), 0, cntQueryCallback, this);
	}
	else
	{
		WTWFUNCTIONS* wtw = PluginController::getInstance().getWTWFUNCTIONS();
		__LOG_F(wtw, WTW_LOG_LEVEL_ERROR, MDL, L"Databes not created");
	}

	TVITEM tvi;
	TVINSERTSTRUCT tvins;

	tvi.mask = TVIF_TEXT|TVIF_PARAM;
	tvins.hParent = TVI_ROOT;
	tvins.hInsertAfter = TVI_FIRST;
	tvi.pszText = L" Ostatnich 100";
	tvi.lParam = -1;
	tvins.item = tvi;
	TreeView_InsertItem(_hTreeView, &tvins);
}

/** Action after single left-click on cotact in tree */
void ArchPage::onCntSel(utfstring& cntId)
{
	if(busy)
		return;
	busy = true;

	ListView_DeleteAllItems(_hListView); // clearing previous descriptions for previous cnt

	std::ostringstream query;

	if(cntId.empty())
	{
		last100 = true;
		query << 
				"SELECT wtw_desc.contact_id, wtw_desc.account_id, wtw_desc.status_code,wtw_desc.desc_text,wtw_desc.change_date,wtw_accounts.net,wtw_accounts.login " \
				"FROM wtw_desc " \
				"INNER JOIN wtw_accounts ON wtw_desc.account_id=wtw_accounts.account_id ";
		if(lastMonth)
			query << "WHERE wtw_desc.change_date > " << (static_cast<__int64>(time(NULL)) - MONTH) << " ";		
		query <<
				"ORDER BY wtw_desc.change_date DESC " \
				"LIMIT 100;";
		ListView_SetColumnWidth(_hListView, 1, 100);
	}
	else
	{
		last100 = false;
		query <<
				"SELECT wtw_desc.contact_id, wtw_desc.account_id, wtw_desc.status_code,wtw_desc.desc_text,wtw_desc.change_date,wtw_accounts.net,wtw_accounts.login " \
				"FROM wtw_desc " \
				"INNER JOIN wtw_accounts ON wtw_desc.account_id=wtw_accounts.account_id " \
				"WHERE wtw_desc.contact_id='" << cntId.toUTF8() << "'";
		if(lastMonth)
			query << " AND wtw_desc.change_date > " << (static_cast<__int64>(time(NULL)) - MONTH) << ";";
		else
			query << ";";
		ListView_SetColumnWidth(_hListView, 1, 0);
	}

	PluginController& pc = PluginController::getInstance(); 
	if (!pc.dtb->executeSQL(query.str().c_str(), msgsQueryCallback, this)) { // getting new desc
		__LOG_F(pc.getWTWFUNCTIONS(), WTW_LOG_LEVEL_ERROR, MDL, 
			L"Could not get descriptions list for %s", cntId.empty() ? L"Last 100" : cntId.toUTF16());
	}

	busy = false;
}

ArchPage::~ArchPage()
{
	TreeView_SetImageList(_hTreeView, NULL, TVSIL_NORMAL); // to prevent deletion of HIMAGELIST contents
	ListView_SetImageList(_hListView, NULL, LVSIL_SMALL);
	DestroyWindow(_hTreeView);
	DestroyWindow(_hListView);
	if(GetProp(_parent,L"ARCH_PREVPROC"))
		SetWindowLongPtr(_parent,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(GetProp(_parent,L"ARCH_PREVPROC")));
	PluginController::getInstance().dtb->busy = NULL;
}

/////////////////////////// CALLBACKS ////////////////////////////////////////

/** Adds a cnt to tree, sqlite callback */
int ArchPage::cntQueryCallback(void *arg, int argc, char **argv, char **azColName) // adding contacts in tree view
{
	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	ProtocolInfo info;

	if(argc!=4)
		return SQLITE_ERROR;
	if(	!argv[0] || // contact_id
		!argv[1] || // contact_net
		!argv[2] ||	// contact_login
		!arg)		// pointer to ArchPage
		return SQLITE_ERROR;

	ArchPage* pAP = reinterpret_cast<ArchPage*>(arg);

	tvi.mask = TVIF_TEXT|TVIF_PARAM; // will be put as a root child, sorted
	if(pAP->_icons)
		tvi.mask |= TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvins.hParent = TVI_ROOT;
	tvins.hInsertAfter = TVI_SORT;

	info.uid.assign(argv[0]);
	info.net.assign(argv[1]);
	
	utfstring login(argv[2]);
	unsigned int i = 0;
	ProtocolInfo prot;
	while(i<pAP->protos->size())
	{
		ProtocolInfo& pi = pAP->protos->at(i);
		if(pi.net == info.net && pi.uid == login)
		{
			prot = pi;
			break;
		}
		i++;
	}
	if (prot.net.empty())
	{
		WTWFUNCTIONS* wtw = PluginController::getInstance().getWTWFUNCTIONS();
		__LOG_F(wtw, WTW_LOG_LEVEL_ERROR, MDL, 
			L"Could not find sid for this protocol %s %s", info.net.toUTF16(), login.toUTF16());
		return SQLITE_ERROR;
	}
	info.sid = prot.sid;

	WTWFUNCTIONS* wtw = PluginController::getInstance().getWTWFUNCTIONS();

	wtwContactDef cntDef; // getting pointer to cnt, to get its display
	cntDef.netClass = info.net.toUTF16();
	cntDef.id = info.uid.toUTF16();
	cntDef.netId = info.sid;
	WTW_PTR hCnt = NULL;
	WTW_PTR ret = wtw->fnCall(WTW_CTL_CONTACT_FIND_EX, cntDef, reinterpret_cast<WTW_PARAM>(&hCnt));

	if(ret == S_OK && hCnt) // if cnt found, we get its display
	{
		wtwContactListItem cntItem;
		wtw->fnCall(WTW_CTL_CONTACT_GET, hCnt,cntItem);
		info.display.assign(cntItem.itemText);
	}
	else // else we set its uid as a display
		info.display.assign(info.uid);

	pAP->_cnts.push_back(info); // list used later by archiveCallback proc

	tvi.pszText = const_cast<wchar_t*>(info.display.toUTF16());
	tvi.lParam = pAP->_cnts.size()-1;
	if(pAP->_icons) // setting the net icon
		tvi.iImage = tvi.iSelectedImage = pAP->_icons->getId(info.net,info.sid,WTW_PRESENCE_ONLINE);
	tvins.item = tvi;
	TreeView_InsertItem(pAP->_hTreeView, &tvins);

	return SQLITE_OK;
}

/** Adds a one cnt description to list, sqlite callback */
int ArchPage::msgsQueryCallback(void *arg, int argc, char **argv, char **azColName) // adding contact descriptions in list view
{
	if(argc!=7)
		return SQLITE_ERROR;
	if(	!argv[0] || // contact_id
		!argv[2] || // status_code
		!argv[3] || // desc_text
		!argv[4] || // change_date
		!argv[5] ||	// net
		!argv[6] || // login
		!arg)		// pointer to ArchPage
		return SQLITE_ERROR;

	ArchPage* pAP = reinterpret_cast<ArchPage*>(arg);

	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	if(pAP->last100)
		lvi.iItem = ListView_GetItemCount(pAP->_hListView);
	else
		lvi.iItem = 0;
	lvi.iSubItem = 0;

	int status = atoi(argv[2]) & 0xF; // converting status to text description of it
	switch(status)
	{
		case WTW_PRESENCE_ONLINE: lvi.pszText = L"Dostępny"; break;
		case WTW_PRESENCE_CHAT: lvi.pszText = L"Porozmawiajmy"; break;
		case WTW_PRESENCE_DND: lvi.pszText = L"Jestem zajęty"; break;
		case WTW_PRESENCE_AWAY: lvi.pszText = L"Zaraz wracam"; break;
		case WTW_PRESENCE_XA: lvi.pszText = L"Wrócę później"; break;
		case WTW_PRESENCE_INV: lvi.pszText = L"Niewidoczny"; break;
		case WTW_PRESENCE_OFFLINE: lvi.pszText = L"Niedostępny"; break;
		default: lvi.pszText = L"Nieznany";
	}

	utfstring netId(argv[5]);
	utfstring login(argv[6]);
	unsigned int i = 0;
	ProtocolInfo prot;
	while(i<pAP->protos->size())
	{
		ProtocolInfo& pi = pAP->protos->at(i);
		if(pi.net == netId && pi.uid == login)
		{
			prot = pi;
			break;
		}
		i++;
	}

	WTWFUNCTIONS* wtw = PluginController::getInstance().getWTWFUNCTIONS();
	if(prot.net.empty())
	{
		__LOG_F(wtw, WTW_LOG_LEVEL_ERROR, MDL, L"Could not find sid for this protocol %s %s", netId.toUTF16(), login.toUTF16());
		return SQLITE_ERROR;
	}

	if(pAP->_icons) // setting the proper status icon 
	{
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = pAP->_icons->getId(netId,prot.sid,status);
	}

	ListView_InsertItem(pAP->_hListView, &lvi);

	// inserting display to list (for all statuses)
	if(pAP->last100)
	{
		// getting pointer to cnt, to get its display
		utfstring uid(argv[0]);
		wtwContactDef cntDef;
		cntDef.netClass = prot.net.toUTF16();
		cntDef.id = uid.toUTF16();
		cntDef.netId = prot.sid;
		WTW_PTR hCnt = NULL;
		WTW_PTR ret = wtw->fnCall(WTW_CTL_CONTACT_FIND_EX, cntDef, reinterpret_cast<WTW_PARAM>(&hCnt));

		if(ret == S_OK && hCnt) // if cnt found, we get its display
		{
			wtwContactListItem cntItem;
			wtw->fnCall(WTW_CTL_CONTACT_GET,hCnt,cntItem);
			uid = cntItem.itemText;
		} // else we set its uid as a display

		ListView_SetItemText(pAP->_hListView, lvi.iItem, 1, const_cast<wchar_t*>(uid.toUTF16()));
	}

	utfstring descText(argv[3]);
	ListView_SetItemText(pAP->_hListView, lvi.iItem, 2, const_cast<wchar_t*>(descText.toUTF16()));
	
	time_t t = (time_t)atof(argv[4]); // setting the time of status change in format HH:MM:SS DD.MM.YYYY
	struct tm* timeinfo = localtime(&t);
	
	utfstring time_str;
	time_str.format(L"%.2d:%.2d:%.2d %.2d.%.2d.%.4d",
		timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,
		timeinfo->tm_mday,timeinfo->tm_mon+1,timeinfo->tm_year+1900);
	ListView_SetItemText(pAP->_hListView, lvi.iItem, 3, const_cast<wchar_t*>(time_str.toUTF16()));

	return SQLITE_OK;
}

/** Parents (received in info->handle) window proc, for receiving WM_NOTIFY for tree and list & WM_COMMAND for copy menu*/
LRESULT WINAPI ArchPage::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ArchPage* pAP = reinterpret_cast<ArchPage*>(GetProp(hWnd,L"ARCHPAGE"));
	if(!pAP)
		return DefWindowProc(hWnd,uMsg,wParam,lParam);

	if(uMsg==WM_NOTIFY)
	{
		switch(((LPNMHDR)lParam)->code)
		{
		case TVN_SELCHANGED: // selection changed on tree with contacts
			{
				LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(lParam);
				int id = static_cast<unsigned int>(pnmtv->itemNew.lParam);
				if(id == -1)
					pAP->onCntSel(utfstring(L""));
				else if(id >= 0 && id < static_cast<int>(pAP->_cnts.size()))
					pAP->onCntSel(pAP->_cnts[id].uid);
				return 0;
			}
		case LVN_GETINFOTIP: // tip on list with descriptions (with full description)
			{
				LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(lParam);
				if(pGetInfoTip)
				{
					wchar_t buff[1024]; // limit of the tooltip is 1024
					LVITEM lvItem;
					lvItem.mask = LVIF_TEXT;
					lvItem.iItem = pGetInfoTip->iItem;
					lvItem.iSubItem = 1;
					lvItem.cchTextMax = 1024;
					lvItem.pszText = buff; 
					if(ListView_GetItem(pAP->_hListView,&lvItem)==TRUE)
						wcscpy_s(pGetInfoTip->pszText,pGetInfoTip->cchTextMax,lvItem.pszText);
				}
				return 0;
			}
		case NM_RCLICK: // copy popup menu on right click on list
			{
				if(((LPNMHDR)lParam)->hwndFrom != pAP->_hListView)
					return 0;
				POINT point;
				GetCursorPos(&point);
				TrackPopupMenu(GetSubMenu(pAP->hLVMenu,0),0,point.x, point.y,0,hWnd,NULL);
				return 0;
			}
		}
	}
	if(uMsg==WM_COMMAND)
	{ 
		switch(LOWORD(wParam))
		{
			case ID_LVCOPY:
				{
					wchar_t buff[1024]; // I'm lazy, this should be dynamic ;)
					LVITEM lvItem;
					lvItem.mask = LVIF_TEXT;
					lvItem.iItem = ListView_GetSelectionMark(pAP->_hListView);
					lvItem.iSubItem = 2;
					lvItem.cchTextMax = 1024;
					lvItem.pszText = buff; 
					if(ListView_GetItem(pAP->_hListView,&lvItem)==TRUE && OpenClipboard(NULL))
					{
						EmptyClipboard();
						HGLOBAL hGlobText = GlobalAlloc(GMEM_MOVEABLE,(wcslen(buff)+1)*sizeof(wchar_t)); 
						if(hGlobText!=NULL)
						{
							HANDLE hStr = GlobalLock(hGlobText);
							memcpy(hStr,buff,(wcslen(buff)+1)*sizeof(wchar_t));
							GlobalUnlock(hStr); 
							SetClipboardData(CF_UNICODETEXT,hStr);
						}
						CloseClipboard();
					}
					return 0;
				}
			case ID_SQLCMD:
				{
					DialogBox(PluginController::getInstance().hInst, MAKEINTRESOURCE(IDD_SQLCMD), hWnd, sqlCmdWndProc);
					return 0;
				}
		}
		if(HIWORD(wParam) == CBN_SELCHANGE)
		{
			if(!pAP->busy)
			{
				LRESULT cIndex = SendMessage(pAP->_hCombo,CB_GETCURSEL,0,0);
				switch(cIndex)
				{
				case 0:
					{
						if(!pAP->lastMonth)
						{
							TreeView_Select(pAP->_hTreeView, NULL, TVGN_CARET);
							pAP->lastMonth = true;
							pAP->updateCnts();
						}
						break;
					}
				case 1:
					{
						if(pAP->lastMonth)
						{
							TreeView_Select(pAP->_hTreeView, NULL, TVGN_CARET);
							pAP->lastMonth = false;
							pAP->updateCnts();
						}
					}
				}
			}
			return 0;
		}
	}
	
	if(GetProp(hWnd,L"ARCH_PREVPROC"))
		return CallWindowProc(reinterpret_cast<WNDPROC>(GetProp(hWnd,L"ARCH_PREVPROC")),hWnd,uMsg,wParam,lParam);
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

}; // namespace archStatusLog

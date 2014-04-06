#include "stdinc.h"
#include "PluginController.h"
#include "Dtb.h"

namespace archStatusLog
{
	HMENU ArchPage::hLVMenu;

	// TODO: WTW_PROTO_FUNC_GET_GUID is not working so it's impossible to retrieve net login
	/*WTW_PTR onProtocolAdd(WTW_PARAM wParam, WTW_PARAM lParam, void * cbData)
	{
		wtwProtocolDef* pd = reinterpret_cast<wtwProtocolDef*>(wParam);
		ProtocolInfo* piElem = protosInfos.find(ProtocolInfo(pd->netClass,pd->netId,NULL,NULL));
		if(pd->netClass && wcsstr(pd->netClass,L"\'")!=NULL)
		{
			__LOG_F(wtw,WTW_LOG_LEVEL_ERROR,MIDL,L"Net id %s contains ' (illegal charcter)",pd->netClass);
			return -1;
		}
		if(pd->protoIconId && wcsstr(pd->protoIconId,L"\'")!=NULL)
		{
			__LOG_F(wtw,WTW_LOG_LEVEL_ERROR,MIDL,L"Icon id %s contains ' (illegal charcter)",pd->protoIconId);
			return -1;
		}

		wchar_t* guid = reinterpret_cast<wchar_t*>(wtw->fnCall(WTW_PROTO_FUNC_GET_GUID,
			reinterpret_cast<WTW_PARAM>(pd->netClass),pd->netId)); 
		if(!guid)
			return -1;
		wchar_t* login; // TODO
		if(!piElem) // if not exists create a new one
		{
			protosInfos.push_back(ProtocolInfo(pd->netClass,pd->netId,NULL,pd->protoIconId));
		}
		else
		{
			if(piElem->icon)
				delete [] piElem->icon;
			piElem->icon = new wchar_t[wcslen(pd->protoIconId)+1];
			wcscpy(piElem->icon,pd->protoIconId);

			if(piElem->uid)
				delete [] piElem->uid;
			piElem->uid = new wchar_t[wcslen(login)+1];
			wcscpy(piElem->uid,login);
		}
		return 0;
	}*/

	int PluginController::onLoad(WTWFUNCTIONS *wtw)
	{
#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

		_wtw = wtw;
		ArchPage::hLVMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_LVPOPUP)); // loading copy menu
		dtb = new Dtb(wtw);

		wtwOptionPageDef pg;

		// adding "Statusy" page to archive window
		pg.id = L"WTW/ArchPage/archStatusLog";
		pg.caption = L"Statusy";
		pg.callback = archiveCallback; 
		pg.cbData = this;
		_wtw->fnCall(WTW_ADD_ARCH_PAGE, reinterpret_cast<WTW_PARAM>(hInst), pg);

		// setting hooks
		_statusHook = _wtw->evHook(WTW_ON_PROTOCOL_EVENT, onStatusChange, this);
		_allPluginsLoadedHook = _wtw->evHook(WTW_EVENT_ALL_PLUGINS_LOADED, enumProtocols, this);
		//_protocolAddHook = _wtw->evHook(WTW_PROTO_EVENT_ON_ADD, onProtocolAdd, NULL);

		// creating tables 
		dtb->executeSQL(
			"BEGIN TRANSACTION;\n" \
			"CREATE TABLE IF NOT EXISTS wtw_desc(contact_id TEXT, account_id INTEGER, status_code INTEGER, desc_text TEXT, change_date INTEGER);\n" \
			"CREATE TABLE IF NOT EXISTS wtw_accounts(account_id INTEGER PRIMARY KEY, net TEXT, login TEXT, UNIQUE (net,login));\n" \
			"COMMIT;",NULL,NULL);
		// UPDATE wtw_desc SET account_id = account_id + 1
		enumProtocols(0, 0, this);

		return 0;
	}

	int PluginController::onUnload()
	{
		wtwOptionPageDef pg;

		// removing "Statusy" page from archive
		pg.id = L"WTW/ArchPage/archStatusLog";
		_wtw->fnCall(WTW_REMOVE_ARCH_PAGES, reinterpret_cast<WTW_PARAM>(hInst), pg);
		
		// removing hooks
		if (_statusHook)
			_wtw->evUnhook(_statusHook);
		if (_allPluginsLoadedHook)
			_wtw->evUnhook(_allPluginsLoadedHook);
		//if(protocolAddHook)
		//	wtw->evUnhook(protocolAddHook);

		// removing used classes if any exists
		if (_archPage)
			delete _archPage;
		if (_protoIcons)
			delete _protoIcons;
		if (ArchPage::hLVMenu)
			DestroyMenu(ArchPage::hLVMenu);
		if(dtb)
			delete dtb;
		return 0;
	}

	// callbacks --------------------------

	WTW_PTR PluginController::onStatusChange(WTW_PARAM wParam, WTW_PARAM lParam, void *cbData)
	{
		PluginController* plugInst = reinterpret_cast<PluginController*>(cbData);
		wtwProtocolEvent *ev = reinterpret_cast<wtwProtocolEvent*>(wParam);
		if(ev->event == WTW_PEV_NETWORK_LOGIN)
		{
			plugInst->dtb->setBufferedSQL(true);
		} 
		else if (ev->event == WTW_PEV_PRESENCE_RECV && ev->type == WTW_PEV_TYPE_AFTER)
		{
			wtwPresenceDef *pres = (wtwPresenceDef*)lParam;
			if(!pres->pContactData->id || wcscmp(pres->pContactData->id, L"") == 0)
			{
				__LOG_F(plugInst->_wtw, WTW_LOG_LEVEL_ERROR, MDL, L"Status change received, but with empty contact id");
				return -1;
			}

			std::vector<ProtocolInfo>::iterator itProt = find(
					plugInst->_protosInfos.begin(), 
					plugInst->_protosInfos.end(), 
					ProtocolInfo(pres->pContactData->netClass, pres->pContactData->netId, NULL, NULL, NULL)
			);
			if (itProt==plugInst->_protosInfos.end())
			{
				__LOG_F(plugInst->_wtw, WTW_LOG_LEVEL_ERROR, MDL, L"Net %s, sid %d not found on the protocol list, status change not added to archive!", pres->pContactData->netClass, pres->pContactData->netId);
				return -1;
			}
			utfstring desc(pres->curDescription ? pres->curDescription : L"");
			desc.replace(L'\'', L"''");
			utfstring query;
			query.format(L"INSERT INTO wtw_desc VALUES ('%s',(SELECT account_id FROM wtw_accounts WHERE net='%s' AND login='%s'),%d,'%s',%u);" // adding to SQL dtb
				, pres->pContactData->id, pres->pContactData->netClass, itProt->uid.toUTF16(), pres->curStatus, desc.toUTF16(), pres->curTimeStamp);
			//__LOG_F(plugInst->_wtw, WTW_LOG_LEVEL_INFO, MDL, query.toUTF16());
			if(plugInst->dtb->isBufferedSQL())
				plugInst->dtb->bufferedExecute(query.toUTF8());
			else
				plugInst->dtb->executeSQL(query.toUTF8(), NULL, NULL);
		}
		return 0;
	}

	WTW_PTR PluginController::enumProtocols(WTW_PARAM wParam, WTW_PARAM lParam, void *cbData)
	{
		// extracting login for each protocol
		PluginController* plugInst = reinterpret_cast<PluginController*>(cbData);
		int protoCount = static_cast<int>(plugInst->_wtw->fnCall(WTW_PROTO_FUNC_ENUM, NULL, -1));
		if (protoCount)
		{
			wtwProtocolInfo *pi = new wtwProtocolInfo[protoCount]; // TODO: change to vector

			plugInst->_wtw->fnCall(WTW_PROTO_FUNC_ENUM, reinterpret_cast<WTW_PARAM>(pi), protoCount);
			for (int i = 0; i < protoCount; i++)
				if (wcscmp(pi[i].netClass, L"META") != 0)
				{
					if (pi[i].name && (wcsstr(pi[i].name, L"(") == NULL || wcsstr(pi[i].name, L")") == NULL))
					{
						__LOG_F(plugInst->_wtw, WTW_LOG_LEVEL_NORMAL, MDL, L"Could not find name in %s for net %s", pi[i].name, pi[i].netClass);
						continue;
					}
					if (pi[i].netClass && wcsstr(pi[i].netClass, L"\'") != NULL)
					{
						__LOG_F(plugInst->_wtw, WTW_LOG_LEVEL_ERROR, MDL, L"Net id %s contains ' (illegal charcter)", pi[i].netClass);
						continue;
					}
					if (pi[i].name && wcsstr(pi[i].name, L"\'") != NULL)
					{
						__LOG_F(plugInst->_wtw, WTW_LOG_LEVEL_ERROR, MDL, L"Login %s contains ' (illegal charcter)", pi[i].name);
						continue;
					}
					wchar_t *login = new wchar_t[wcslen(pi[i].name)];
					swscanf(pi[i].name, L"%*s (%s)", login); // getting login from "NET_NAME (LOGIN)"
					login[wcslen(login) - 1] = 0; // cut last ")"
					plugInst->_protosInfos.push_back(ProtocolInfo(pi[i].netClass, pi[i].netId, login, NULL, pi[i].protoIconId));
					delete [] login;
				}
				delete [] pi;
				// adding any new protocols
				for (unsigned int i = 0; i < plugInst->_protosInfos.size(); i++)
				{
					utfstring query;
					query.format(L"INSERT INTO wtw_accounts VALUES (NULL,'%s','%s');", plugInst->_protosInfos[i].net.toUTF16(), plugInst->_protosInfos[i].uid.toUTF16());
					plugInst->dtb->executeSQL(query.toUTF8(), NULL, NULL);
				}
		}
		if (plugInst->_protoIcons)
			delete plugInst->_protoIcons;
		plugInst->_protoIcons = new ProtoIcons(plugInst->_wtw, &plugInst->_protosInfos); // giving the information about all protocols to ProtocolsIcons
		return 0;
	}

	WTW_PTR PluginController::archiveCallback(WTW_PARAM wParam, WTW_PARAM lParam, void *cbData)
	{
		PluginController* plugInst = reinterpret_cast<PluginController*>(cbData);
		wtwOptionPageShowInfo *info = reinterpret_cast<wtwOptionPageShowInfo*>(wParam);
		wcscpy(info->windowCaption, L"Statusy");
		wcscpy(info->windowDescrip, L"Zmiany statusów i opisów");
		if (!plugInst->_archPage && info->action != WTW_ARCH_PAGE_ACTION_DESTROY)
			plugInst->_archPage = new ArchPage(plugInst->_protoIcons, &plugInst->_protosInfos, info->x, info->y, info->cx, info->cy, info->handle);
		switch (info->action)
		{
		case WTW_ARCH_PAGE_ACTION_SHOW:
			{
				plugInst->_archPage->show();
				break;
			}
		case WTW_ARCH_PAGE_ACTION_MOVE:
			{
				plugInst->_archPage->move(info->x, info->y, info->cx, info->cy);
				break;
			}
		case WTW_ARCH_PAGE_ACTION_HIDE:
			{
				plugInst->_archPage->hide();
				break;
			}
		case WTW_ARCH_PAGE_ACTION_DESTROY:
			{
				if(plugInst->_archPage)
				{
					delete plugInst->_archPage;
					plugInst->_archPage = NULL;
				}
			}
		}
		return 0;
	}

};
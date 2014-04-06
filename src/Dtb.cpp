#pragma once

#include "stdinc.h"
#include "Dtb.h"

namespace archStatusLog
{
	DWORD Dtb::SQLExecThreadFunc(LPVOID arg)
	{
		Dtb* dtb = static_cast<Dtb*>(arg);
		PluginController &plugInst = PluginController::getInstance();
		if(dtb->busy)
			*(dtb->busy) = true;

		Sleep(dtb->beDelay); // we are waiting delay msec for incoming queries

		if (!dtb->toExecute.empty()) 
		{
			std::ostringstream sql;
			/*bool singleSelect = (dtb->toExecute.size() == 1 && 
				(strUtils::startsWith(dtb->toExecute.top(), "select") 
				|| strUtils::startsWith(dtb->toExecute.top(), "SELECT")));
			if (!singleSelect)
				sql << "BEGIN;\n";*/
			while (!dtb->toExecute.empty())
			{
				sql << dtb->toExecute.top() << "\n";
				dtb->toExecute.pop();
			}
			/*if (!singleSelect)
				sql << "COMMIT;";*/

			/* __LOG_F(PluginController::getInstance().getWTWFUNCTIONS(), WTW_LOG_LEVEL_INFO, MDL, 
				my_wstring(sql.str().c_str()).toUTF16()); */
			char *zErrMsg;
			int res = sqlite3_exec(dtb->db, sql.str().c_str(), dtb->beClbk, dtb->beArg, &zErrMsg);
			if (res != SQLITE_OK)
			{
				utfstring errMsg(zErrMsg);
				// no longer needed, WTW is logging this automatically
				//__LOG_F(plugInst.getWTWFUNCTIONS(), WTW_LOG_LEVEL_ERROR, MDL, L"Sqlite exec error - %d %s", res, errMsg.toUTF16());
				sqlite3_free(zErrMsg);
			}
		}

		if(dtb->hBeWait)
		{
			EndDialog(dtb->hBeWait, 0);
			dtb->hBeWait = NULL;
		}

		dtb->shouldBeBuffered = false;
		CloseHandle(dtb->hThread);
		dtb->hThread = NULL;
		if(dtb->busy)
			*(dtb->busy) = false;
		
		return 0;
	}

	Dtb::Dtb(WTWFUNCTIONS *wtw) : hThread(NULL), shouldBeBuffered(false), hBeWait(NULL), busy(NULL)
	{
		utfstring archPath;
		wtwDirectoryInfo dirInfo;
		
		dirInfo.dirType = WTW_DIRECTORY_ARCHIVE;
		dirInfo.flags = WTW_DIRECTORY_FLAG_FULLPATH;
		dirInfo.bi.bufferSize = 512;
		dirInfo.bi.pBuffer = new wchar_t[512];
				
		wtw->fnCall(WTW_GET_DIRECTORY_LOCATION, reinterpret_cast<WTW_PARAM>(&dirInfo), NULL);
		archPath.format(L"%sstatuses.sq3", dirInfo.bi.pBuffer);
		delete [] dirInfo.bi.pBuffer;

		int res = sqlite3_open16(archPath.toUTF16(), &db);
		if (res)
		{
			__LOG_F(wtw, WTW_LOG_LEVEL_ERROR, MDL, L"Sqlite open error - %d", res);
			sqlite3_close(db);
			db = NULL;
		}
	}

	Dtb::~Dtb()
	{
		if (db)
			sqlite3_close(db);
	}
	
	void Dtb::bufferedExecute(char const*query, int delay, tSQLcallback callback, void* arg)
	{
		DWORD id;
		toExecute.push(query);
		if (!hThread)
		{
			beClbk = callback;
			beArg = arg;
			beDelay = delay;
			hThread = CreateThread(NULL, 0, SQLExecThreadFunc, this, 0, &id);
		}
	}

	bool Dtb::executeSQL(char const*query, tSQLcallback callback, void *arg)
	{		
		//__LOG_F(PluginController::getInstance().getWTWFUNCTIONS(), WTW_LOG_LEVEL_INFO, MDL, my_wstring(query).toUTF16());
		int res = sqlite3_exec(db, query, callback, arg, &zErrMsg);
		if (res != SQLITE_OK)
		{
			utfstring errMsg(zErrMsg);
			const wchar_t* msg = errMsg.toUTF16();
			// no longer needed, WTW is logging this automatically
			//__LOG_F(PluginController::getInstance().getWTWFUNCTIONS(), WTW_LOG_LEVEL_ERROR, MDL, L"Sqlite execution error - %d %s", res, msg);
			sqlite3_free(zErrMsg);
		}
		if (res != SQLITE_OK)
			return false;
		return true;
	}
}

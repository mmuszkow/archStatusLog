#pragma once

#include "stdinc.h"
#include "PluginController.h"
#include "Dtb.h"
#include "SqlCmdWnd.h"

namespace archStatusLog 
{

	INT_PTR CALLBACK sqlCmdWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_INITDIALOG:
			{
				return (INT_PTR)TRUE;
			}
		case WM_COMMAND:
			{
				switch(LOWORD(wParam))
				{
				case IDCLOSE:
					EndDialog(hWnd, 0);
					break;
				case IDC_CLEAR:
					SetDlgItemText(hWnd, IDC_SCRIPT, 0);
					break;
				case IDC_EXECUTE:
					{
						SetDlgItemText(hWnd, IDC_RESULT, 0);

						int len = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_SCRIPT));
						wchar_t* sql = new wchar_t[len+2];
						GetDlgItemTextW(hWnd, IDC_SCRIPT, sql, len+1);
						utfstring str(sql);
						PluginController::getInstance().dtb->executeSQL(str.toUTF8(), sqlQueryCallback, GetDlgItem(hWnd, IDC_RESULT));
						delete [] sql;
					}
					break;
				}
				return (INT_PTR)TRUE;
			}
		}
		return (INT_PTR)FALSE;
	}

	int sqlQueryCallback(void *arg, int argc, char **argv, char **azColName)
	{
		HWND hRes = static_cast<HWND>(arg);
		
		std::ostringstream ss;
		int len = GetWindowTextLengthW(hRes);
		if(len < 4096)
		{
			if(len > 0)
			{
				wchar_t* prev = new wchar_t[len+2];
				GetWindowTextW(hRes, prev, len+1);
				utfstring wPrev(prev);
				delete [] prev;
				ss << wPrev.toUTF8() << "\r\n";
			}
		}
		else
			return SQLITE_TOOBIG;

		for(int i=0; i<argc; i++)
		{
			if(azColName[i])
				ss << azColName[i] << ": " << argv[i] << "\r\n";
		}

		utfstring ws(ss.str().c_str());
		SetWindowText(hRes, ws.toUTF16());

		return SQLITE_OK;
	}
};

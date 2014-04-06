#pragma once

namespace archStatusLog {

	static const int ID_SQLCMD = 1627;

	INT_PTR CALLBACK sqlCmdWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int sqlQueryCallback(void *arg, int argc, char **argv, char **azColName);
};

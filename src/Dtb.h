#pragma once

#include "stdinc.h"
#include "PluginController.h"

namespace archStatusLog {
class Dtb
{
public:
	typedef int (*tSQLcallback)(void*,int,char**,char**);
private:
    sqlite3*		db;
    char*			zErrMsg;

	/** Buffered execute is in new thread */
	HANDLE			hThread;
	/** List of commands to execute simultaniously in new thread */
	std::stack<std::string>	toExecute;
	/** If checked all the collected statuses will be added to dtb after 3 sec */
	bool			shouldBeBuffered;
	/** Buffered execute callback */
	tSQLcallback	beClbk;
	/** Buffered execute callback arg */
	void*			beArg;
	/** Time for gathering cmds */
	int				beDelay;

	Dtb() {}
	static DWORD WINAPI SQLExecThreadFunc(LPVOID arg);
public:
	/** Window with "please wait" if used */
	HWND			hBeWait;
	/** Business status (only for buffered) */
	bool*			busy;

	Dtb(WTWFUNCTIONS *wtw);
	~Dtb();
	void bufferedExecute(char const*query, int delay = 3000, tSQLcallback callback = NULL, void* arg = NULL);
	bool executeSQL(char const*query, tSQLcallback callback, void *arg);

	inline void setBufferedSQL(bool val)
	{
		shouldBeBuffered = true;
	}

	inline bool isBufferedSQL() const
	{
		return shouldBeBuffered;
	}
};
}; // namespace archStatusLog

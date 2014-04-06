#pragma once

#include "stdinc.h"
#include "ProtoIcons.h"

namespace archStatusLog
{
	/** Page in archive */
	class ArchPage
	{
		/** Tree with contacts handle */
		HWND	_hTreeView;
		/** List with contact descriptions handle */
		HWND	_hListView;
		/** SQL console button */
		HWND	_hSQL;
		/** Time length combo */
		HWND	_hCombo;
		/** One radio height */
		static const int COMBO_H = 18;
		/** List view width */
		static const int LIST_W = 200;
		/** Distance from borders */
		static const int MARGIN = 10;
		/** Vector contatining additional info about contacts in tree, index is in LPARAM of LVITEM */
		std::vector<ProtocolInfo> _cnts;
		/** Icons for all statuses of all protocols */
		ProtoIcons* _icons;
		/** Protocols list for getting SID */
		std::vector<ProtocolInfo>* protos;
		/** Guess ... */
		HWND	_parent;
		/** If true only statuses from last 30 days will be processed */
		bool	lastMonth;
		static const int MONTH = 2592000;
		/** Internal, if set to true, to Contact column it's display will be added */
		bool	last100;
		/** Contact list is updated only on 1 show */
		bool	firstShow;
		/**	Do one thing at the time */
		bool	busy;

		/** Adds a cnt to tree, sqlite callback */
		static int cntQueryCallback(void *arg, int argc, char **argv, char **azColName);

		/** Adds a one cnt description to list, sqlite callback */
		static int msgsQueryCallback(void *arg, int argc, char **argv, char **azColName) ;

		/** Parents (received in info->handle) window proc, for receiving WM_NOTIFY for 
			tree and list & WM_COMMAND for copy menu
			
			Prop "ARCH_PREVPROC" with pointer to our ArchPage must be set before subclassing 
			the parent window */
		static LRESULT WINAPI wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/** Action after single left-click on cotact in tree */
		void onCntSel(utfstring& cntId);

		/** Update contact list in treeview (used in show) */
		void updateCnts();

		void createWaitWindow();
	public:
		/** Copy popup menu handle */
		static	HMENU			hLVMenu;

		ArchPage(ProtoIcons* icons, const std::vector<ProtocolInfo>* _protos, int x, int y, int cx, int cy, HWND parent);
		~ArchPage();

		void show();
		void hide();
		void move(int x, int y, int cx, int cy);
	};
};
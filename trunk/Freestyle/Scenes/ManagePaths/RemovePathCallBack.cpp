#include "stdafx.h"

#include "RemovePathCallBack.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/ContentList/ContentManager.h"



RemovePathCallBack::RemovePathCallBack() {
}
RemovePathCallBack::~RemovePathCallBack() {
}

void RemovePathCallBack::Start()
{
	FSDSql::getInstance().deleteScanPath(m_CurrentItem.PathId);
	ContentManager::getInstance().RemoveScanPath(m_CurrentItem);
	done = true;
}
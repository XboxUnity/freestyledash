
#include <queue>

class CQueue {
private:
	std::queue<void *> m_objectQueue;
	CRITICAL_SECTION	m_csQueue;
	HANDLE	m_hEvent;
	HANDLE m_hSemophore;

public:
	CQueue();
	~CQueue();
	void PushData(void * queueData);
	void * PopData( void );
	
	// Helper Function
	DWORD GetQueueSize( void );
	HANDLE GetEvent( void );
	void ClearQueue( void );
};
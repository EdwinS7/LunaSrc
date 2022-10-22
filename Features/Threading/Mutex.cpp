#include "mutex.h"

CMutex::CMutex() {
	::InitializeCriticalSection(&m_Lock);
}

CMutex::~CMutex() {
	::DeleteCriticalSection(&m_Lock);
}

void CMutex::Lock() {
	::EnterCriticalSection(&m_Lock);
}

void CMutex::Unlock() {
	::LeaveCriticalSection(&m_Lock);
}
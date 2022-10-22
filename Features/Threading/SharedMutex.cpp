#include "SharedMutex.h"
#include <windows.h>

CSharedMutex::CSharedMutex() {
	::InitializeSRWLock(&lock);
}

CSharedMutex::~CSharedMutex() {
	//No release function
}

void CSharedMutex::rlock() {
	::AcquireSRWLockShared(&lock);
}

bool CSharedMutex::tryrlock() {
	return ::TryAcquireSRWLockShared(&lock);
}

void CSharedMutex::runlock() {
	::ReleaseSRWLockShared(&lock);
}

void CSharedMutex::wlock() {
	::AcquireSRWLockExclusive(&lock);
}

bool CSharedMutex::trywlock() {
	return ::TryAcquireSRWLockExclusive(&lock);
}

void CSharedMutex::wunlock() {
	::ReleaseSRWLockExclusive(&lock);
}

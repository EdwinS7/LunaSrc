
#define SHARED_MUTEX_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class CSharedMutex {
public:
	CSharedMutex();
	~CSharedMutex();
	void rlock();
	bool tryrlock();
	void runlock();
	void wlock();
	bool trywlock();
	void wunlock();
private:
	SRWLOCK lock;
};

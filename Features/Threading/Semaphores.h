#define SEMAPHORES_H
#define WINCLUDES_H
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <stdint.h>

#include <Windows.h>
#include <intrin.h>

class CSemaphore
{
public:
	CSemaphore(bool m_bShared = false);
	~CSemaphore();

	virtual void Wait();
	virtual int TimedWait(size_t milliseconds);
	virtual void Post();
	virtual unsigned long Count();
private:
	HANDLE m_Semaphore;
};

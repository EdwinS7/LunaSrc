#include "semaphores.h"

CSemaphore::CSemaphore(bool m_bShared)
{
	m_Semaphore = CreateSemaphoreA(nullptr, 0, 0xffff, nullptr);
}

CSemaphore::~CSemaphore()
{
	CloseHandle(m_Semaphore);
}

void CSemaphore::Wait()
{
	WaitForSingleObject(m_Semaphore, INFINITE);
}

int CSemaphore::TimedWait(size_t milliseconds)
{
	if (WaitForSingleObject(m_Semaphore, milliseconds) == WAIT_OBJECT_0)
		return 0;

	return 1;
}

void CSemaphore::Post()
{
	ReleaseSemaphore(m_Semaphore, 1, NULL);
}

unsigned long CSemaphore::Count()
{
	long previous;
	switch (WaitForSingleObject(m_Semaphore, 0))
	{
	case WAIT_OBJECT_0:
		ReleaseSemaphore(m_Semaphore, 1, &previous);
		return previous + 1;
	case WAIT_TIMEOUT:
		return 0;
	}
	return 0;
}
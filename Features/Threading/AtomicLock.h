#define ATOMIC_LOCK_H
#include <atomic>

class AtomicLock
{
public:
	AtomicLock();
	~AtomicLock();
	void Lock();
	bool TryLock();
	void Unlock();
private:
	std::atomic_flag m_Lock;
};

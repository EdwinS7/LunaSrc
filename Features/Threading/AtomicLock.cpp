#include "AtomicLock.h"

AtomicLock::AtomicLock()
{
	m_Lock.clear();
}

AtomicLock::~AtomicLock()
{
}

void AtomicLock::Lock()
{
	while (m_Lock.test_and_set(std::memory_order_acquire))
		;
}

bool AtomicLock::TryLock()
{
	return !m_Lock.test_and_set(std::memory_order_acquire);
}

void AtomicLock::Unlock()
{
	m_Lock.clear(std::memory_order_release);
}

#define THREADING_H

#define G_DEFINES_H

template<typename T>
inline constexpr T x64x32(T x64, T x32)
{
	if (sizeof(void*) == 0x8)
		return x64;
	return x32;
}

#define COMMA ,

#include <string.h>
#include "mutex.h"
#include "SharedMutex.h"
#include "semaphores.h"
#include "packedheap.h"
#include "atomiclock.h"
#include <atomic>
#include <Windows.h>
#include <Psapi.h>
#include "stdint.h"

typedef int (*ThreadIDFn)(void);
extern ThreadIDFn AllocateThreadID;
extern ThreadIDFn FreeThreadID;

static CSemaphore dispatchSem;
static CSemaphore waitSem;
static CSharedMutex smtx;

template<typename T, T& Fn>
static void AllThreadsStub(void*)
{
	dispatchSem.Post();
	smtx.rlock();
	smtx.runlock();
	Fn();
}

template<typename T, T& Fn>
static void DispatchToAllThreads(void* data)
{
	smtx.wlock();

	for (size_t i = 0; i < MultiThread::numThreads; i++)
		MultiThread::QueueJobRef(AllThreadsStub<T, Fn>, data);

	for (size_t i = 0; i < MultiThread::numThreads; i++)
		dispatchSem.Wait();

	smtx.wunlock();
	MultiThread::FinishQueue(false);
}

#define SECTION(sec) __declspec(allocate(sec))
#define WSECTION(sec) SECTION(sec)
#define CLZ(x) __builtin_clz(x)
#define OLin(Linux)
#define OWin(Windows) Windows
#define PosixWin(Posix, Windows) Windows
#define LWM(Linux, Windows, Mac) Windows
#define OMac(Mac)
#define OPosix(Posix)
#define paddr(handle, name) GetProcAddress(handle, name)
#define FASTARGS [[maybe_unused]] void* thisptr, [[maybe_unused]] void* edx
#define CFASTARGS thisptr, edx
#define STDARGS
#define THISARGS [[maybe_unused]] void* thisptr
#define LC
#define PC
#define WC COMMA
#define _noinline __declspec(noinline)
#define FRAME_POINTER() (void*)((void**)_AddressOfReturnAddress() - 1)

typedef unsigned long thread_t;


typedef void(*JobFn)(void*);
typedef void* (__stdcall* threadFn)(void*);

struct Job
{
	JobFn function;
	void* args;
	bool ref;
	uint64_t id;

	Job()
	{
		function = nullptr;
		args = nullptr;
		ref = true;
		id = ~0ull;
	}
};

template <typename T>
struct LList
{

	struct LEntry
	{
		T entry;
		idx_t prev;
		idx_t next;
	};

	PackedHeapL<LEntry> entries;

	CMutex lock;
	bool quit;
	idx_t front;
	idx_t back;
	uint64_t lastID;
	uint64_t lastPopID;

	CSemaphore sem;

	LList() {
		front = 0;
		back = 0;
		lastID = 0;
		lastPopID = 0;
	}

	uint64_t Enqueue(const T& data, bool priority = false) {
		lock.Lock();
		idx_t entry = entries.Alloc();
		if (priority) {
			entries[entry] = { data, front, 0 };
			entries[entry].entry.id = lastID;
			if (front)
				entries[front].next = entry;
			if (!back) {
				back = entry;
				entries[back].next = 0;
			}
			entries[entry].prev = front;
			front = entry;
		}
		else {
			entries[entry] = { data, 0, back };
			entries[entry].entry.id = lastID;
			if (back)
				entries[back].prev = entry;
			if (!front) {
				front = entry;
				entries[front].prev = 0;
			}
			entries[entry].next = back;
			back = entry;
		}
		uint64_t id = lastID++;
		lock.Unlock();
		sem.Post();
		return id;
	}

	T DoPopFront(CMutex* lck) {
		lock.Lock();
		if (!front) {
			lock.Unlock();
			return Job();
		}
		if (lck)
			lck->Lock();
		LEntry* entry = &entries[front];
		front = entry->prev;
		if (front)
			entries[front].next = 0;
		else
			back = 0;
		T ret = entry->entry;
		lastPopID = ret.id;
		entries.Free(entry);
		lock.Unlock();
		return ret;
	}

	T PopFront(CMutex* lck = nullptr) {
		sem.Wait();
		if (quit) {
			sem.Post();
			return Job();
		}
		return DoPopFront(lck);
	}

	T TryPopFront() {
		if (sem.TimedWait(0))
			return Job();

		if (quit) {
			sem.Post();
			return Job();
		}

		return DoPopFront(nullptr);
	}

#ifdef _MSC_VER
	__declspec(noinline)
#else
	__attribute__((noinline))
#endif
		bool IsEmpty()
	{
		static volatile short cnt = 0;
		cnt++;
		return !front;
	}
};

struct JobThread
{
	std::atomic_bool shouldQuit;
	std::atomic_bool isRunning;
	std::atomic_bool queueEmpty;
	CMutex* jLock;
	LList<struct Job>* jobs;
	int id;
	void* handle;
};

namespace MultiThread
{
	extern unsigned int numThreads;
	extern thread_local int threadID;
	uint64_t _QueueJob(JobFn function, void* data, bool ref = false, bool priority = false);
	void InitThreads();
	int EndThreads();
	void FinishQueue(bool executeJobs = false);
	JobThread* BindThread(LList<struct Job>* jobsQueue);
	void UnbindThread(LList<struct Job>* jobsQueue);
	thread_t StartThread(threadFn start, void* param, bool detached = true);
	thread_t StartThread(threadFn start, void* param, bool detached, thread_t* thread);
	void JoinThread(thread_t thread, void** returnVal);

	template<typename N, typename T>
	uint64_t QueueJob(N function, T data, bool priority = false) {
		void* d = malloc(sizeof(T));
		memcpy(d, (void*)&data, sizeof(T));
		return _QueueJob((JobFn)function, d, false, priority);
	}

	template<typename N, typename T>
	uint64_t QueueJobRef(N function, T* data, bool priority = false) {
		return _QueueJob((JobFn)function, (void*)data, true, priority);
	}
}

#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include <windows.h>
#include <cstring>

class CCriticalSection {
public:
 #if 0
    CCriticalSection() { std::memset(&cs_, 0, sizeof cs_); ::InitializeCriticalSection(&cs_); }
    ~CCriticalSection() { ::DeleteCriticalSection(&cs_); }
 #else  // グローバルインスタンスのグローバルコンストラクタ実行を避けるためこちらに.
    void create() { std::memset(&cs_, 0, sizeof cs_); ::InitializeCriticalSection(&cs_); }
    void release() { ::DeleteCriticalSection(&cs_); }
 #endif
private:
    friend class CCriticalSectionLock;
    void lock() { EnterCriticalSection(&cs_); }
    void unlock() { LeaveCriticalSection(&cs_); }

private:
    CRITICAL_SECTION    cs_;
};

class CCriticalSectionLock {
public:
    CCriticalSectionLock(CCriticalSection& cs) : rCS_(cs) { rCS_.lock(); }
    ~CCriticalSectionLock() { rCS_.unlock(); }
private:
    void operator=(CCriticalSectionLock const&) {}
private:
    CCriticalSection&   rCS_;
};

#endif  // CRITICALSECTION_H

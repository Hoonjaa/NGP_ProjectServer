#pragma once

#include <winsock2.h>
#include <windows.h>
#include <queue>
#include <system_error>

template <typename T>
class JobContainer {
public:
    JobContainer()
    {
		ready_ = CreateEventW(nullptr, FALSE, FALSE, nullptr); // 이벤트 생성, 자동 리셋, 초기 상태 비신호
        if (ready_ == nullptr)
            throw std::system_error(GetLastError(), std::system_category(), "CreateEventW"); 

		InitializeCriticalSection(&section_); // 크리티컬 섹션 초기화
    }

    ~JobContainer()
    {
        CloseHandle(ready_);
        DeleteCriticalSection(&section_);
    }

	JobContainer(const JobContainer&) = delete; // 복사 생성자 삭제
	JobContainer& operator=(const JobContainer&) = delete; // 복사 대입 연산자 삭제
	// 큐 객체를 복사할 수 없도록 복사 생성자와 복사 대입 연산자를 삭제

	bool job_push(const T& job) // 작업을 큐에 추가
    {
        {
			Lock lock(section_); // 크리티컬 섹션 진입
            if (stopped_)
                return false;

            jobs_.push(job); 
        }
		SetEvent(ready_); // 이벤트를 신호 상태로 설정하여 대기 중인 스레드를 깨움
        return true;
    }

    bool TryPop(T& job)
    {
		Lock lock(section_);
        return pop_front(job);
    }

	bool WaitPop(T& job) // 큐가 비어있으면 대기, 큐에 작업이 있으면 제거하고 job에 할당
    {
        for (;;) { 
            {
                Lock lock(section_);
                if (pop_front(job))
                    return true;
                if (stopped_)
                    return false;
            }

			if (WaitForSingleObject(ready_, INFINITE) == WAIT_FAILED) // 이벤트가 신호 상태가 될 때까지 대기, 실패 시 예외 발생
                throw std::system_error(GetLastError(), std::system_category(), "WaitForSingleObject");
        }
    }

	bool job_pop(T& job) // 작업을 큐에서 제거하고 job에 할당, 큐가 비어있으면 대기
    {
        return WaitPop(job);
    }

	void Stop() // 서버 종료 시 호출, 큐에 남아있는 작업을 모두 처리한 후 스레드 종료
    {
        {
            Lock lock(section_);
            stopped_ = true;
        }
        SetEvent(ready_);
    }

private:
	class Lock { // 크리티컬 섹션을 관리
    public:
        explicit Lock(CRITICAL_SECTION& section) : section_(section)
        {
            EnterCriticalSection(&section_);
        }

        ~Lock()
        {
            LeaveCriticalSection(&section_);
        }

        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;

    private:
        CRITICAL_SECTION& section_;
    };

	bool pop_front(T& job) // 앞의 작업을 큐에서 제거하고 job에 할당, 큐가 비어있으면 false 반환
    {
        if (jobs_.empty())
            return false;

        job = jobs_.front();
        jobs_.pop();
        return true;
    }

    std::queue<T> jobs_;
    CRITICAL_SECTION section_;
    HANDLE ready_;
    bool stopped_ = false;
};

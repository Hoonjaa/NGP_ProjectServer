#include "Header/Common.h"

template <typename T>
class JobContainer {
private:
	std::queue<T> Container;
	CRITICAL_SECTION cs;
	CONDITION_VARIABLE cv;

public:
	JobContainer() { // 생성자
		InitializeCriticalSection(&cs);
		InitializeConditionVariable(&cv);
	}

	~JobContainer() { // 소멸자
		DeleteCriticalSection(&cs);
	}

	void job_push(const T& job) // 작업 추가
	{
		EnterCriticalSection(&cs);
		Container.push(job);
		LeaveCriticalSection(&cs);
	}

	void job_pop(T& job) // 작업 제거
	{
		EnterCriticalSection(&cs);
		while (Container.empty()) {
			SleepConditionVariableCS(&cv, &cs, INFINITE); // 나중에 틱 시간으로 교체.
		}
		job = Container.front();
		Container.pop();
		LeaveCriticalSection(&cs);
	}
};
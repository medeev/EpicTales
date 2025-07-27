
#pragma once

#include <atomic>

class BusyState
{
protected:
	std::atomic_int _count;
public:
	enum class EState
	{
		kIdle,
		kBusy
	};
	void increase() {
		_count.fetch_add(1);
	}
	void decrease() {
		_count.fetch_sub(1);
	}
	EState getState() const {
		return (_count.load() > 0) ? EState::kBusy : EState::kIdle;
	}

};

typedef std::shared_ptr<BusyState> BusyStatePtr;

class BusyStateUpdater
{
	BusyStateUpdater() = delete;
	BusyStateUpdater& operator=(const BusyStateUpdater&) = delete;
protected:
	BusyStatePtr _busyState;

public:
	BusyStateUpdater(const BusyStatePtr& busyState) : _busyState(busyState)
	{
		if (_busyState)
			_busyState->increase();
	}
	~BusyStateUpdater()
	{
		if (_busyState)
			_busyState->decrease();
	}
};

typedef std::shared_ptr<BusyStateUpdater> BusyStateUpdaterPtr;
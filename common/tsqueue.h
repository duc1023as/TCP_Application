#ifndef TSQUEUE_H
#define TSQUEUE_H

#include<deque>
#include<mutex>
#include<condition_variable>

template<typename T>
class tsqueue
{
public:
	tsqueue() = default;
	tsqueue(const tsqueue<T>&) = delete;
	virtual ~tsqueue() { clear(); }

public:
	// Returns and maintains item at front of Queue
	const T& front()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.front();
	}

	// Returns and maintains item at back of Queue
	const T& back()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.back();
	}

	// Removes and returns item from front of Queue
	T pop_front()
	{
		std::scoped_lock lock(muxQueue);
		auto t = std::move(deqQueue.front());
		deqQueue.pop_front();
		return t;
	}

	// Removes and returns item from back of Queue
	T pop_back()
	{
		std::scoped_lock lock(muxQueue);
		auto t = std::move(deqQueue.back());
		deqQueue.pop_back();
		return t;
	}

	// Adds an item to back of Queue
	void push_back(const T& item)
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.emplace_back(std::move(item));

		std::unique_lock<std::mutex> ul(muxBlocking);
		cvBlocking.notify_one();
	}

	// Adds an item to front of Queue
	void push_front(const T& item)
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.emplace_front(std::move(item));

		std::unique_lock<std::mutex> ul(muxBlocking);
		cvBlocking.notify_one();
	}

	// Returns true if Queue has no items
	bool empty()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.empty();
	}

	// Returns number of items in Queue
	size_t count()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.size();
	}

	// Clears Queue
	void clear()
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.clear();
	}

	void wait()
	{
		while (empty())
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.wait(ul);
		}
	}

protected:
	std::mutex muxQueue;//comment muxQueue 123
	std::deque<T> deqQueue;//comment 2
	std::condition_variable cvBlocking;//comment 4
	std::mutex muxBlocking;
};

#endif
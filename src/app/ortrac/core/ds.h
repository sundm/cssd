#pragma once

#include <vector>

template <typename T>
class CycleQueue
{
public:
	CycleQueue(unsigned size)
		: _size(size),
		_front(0),
		_rear(0) {
		_data = new T[size];
	}

	~CycleQueue() {
		delete[] _data;
	}

	bool isEmpty() {
		return _front == _rear;
	}

	bool isFull() {
		return _front == (_rear + 1) % _size;
	}

	void enqueue(const T &t) {
		if (isFull())
		{
			throw bad_exception();
		}
		_data[_rear] = t;
		_rear = (_rear + 1) % _size;
	}

	T dequeue() {
		if (isEmpty()) {
			throw bad_exception();
		}
		T tmp = _data[_front];
		_front = (_front + 1) % _size;
		return tmp;
	}

private:
	unsigned int _size;
	int _front;
	int _rear;
	T*  _data;
};

template <typename T>
class Circulator
{
public:
	Circulator() = default;

	Circulator(std::vector<T> &source, size_t size = -1) {
		if (-1 == size || source.size() == size) {
			source.swap(_data);
		}
		else { // T must has move semantics
			   /*for_each(
			   source.begin(),
			   -1 == size ? source.end() : source.begin() + size,
			   [this](T &elem) { _data.emplace_back(elem); }
			   );*/
			_data.reserve(std::min(source.size(), size));
			auto end = std::next(source.begin(), _data.capacity());
			std::move(source.begin(), end, std::back_inserter(_data));
		}
	}

	bool isEmpty() const {
		return _data.empty();
	}

	Circulator& operator=(std::vector<T> &source) {
		source.swap(_data);
		return *this;
	}

	Circulator& operator++() {
		if (!_data.empty()) { // x%0 is undefined in c++
			c = (c + 1) % _data.size();
		}
		return *this;
	}

	T& operator*() {
		return _data.at(c); // if data is empty, at() throws, std::optional is an alternative
	}

private:
	int c = 0;
	std::vector<T> _data;
};
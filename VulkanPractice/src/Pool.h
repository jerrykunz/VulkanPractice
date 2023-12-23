#pragma once
#include <vector>

struct FreeChunk
{
	size_t start;
	size_t end;
};

template <typename T, size_t S>
class Pool
{
	std::vector<T> _pool;
	std::vector<FreeChunk> _freeChunks;

	Pool()
	{
		_pool.reserve(S);
	}

	void AddObject(T obj)
	{
		if (_freeChunks.empty())
		{
			_pool.push_back(obj);
			return;
		}

		FreeChunk chunk = _freeChunks.back();
		_pool[chunk.start] = obj;
		chunk.start++;

		if (chunk.start >= chunk.end)
		{
			_freeChunks.pop_back();
		}
	}

	void RemoveObject(size_t index)
	{
		if (_freeChunks.size() <= 0)
		{
			_freeChunks.push_back(index, index);
			return;
		}

		for (FreeChunk& chunk : _freeChunks)
		{
			if (index == chunk.start - 1)
			{
				chunk.start--;
				return;
			}

			if (index == chunk.end + 1)
			{
				chunk.end++;
				return;
			}
		}

		_freeChunks.push_back({ index, index });
	}
};
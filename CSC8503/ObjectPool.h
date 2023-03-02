#pragma once

#include <iostream>
#include <list>

template<class Object>
class ObjectPool
{
public:
	ObjectPool(size_t Size)
	{
		_nSize = Size;
		for (size_t n = 0; n < _nSize; n++)
		{
			_mPool.push_back(new Object());
		}
	}
	virtual ~ObjectPool()
	{
		auto iter = _mPool.begin();
		while (iter != _mPool.end())
		{
			delete* iter;
			++iter;
		}
		_nSize = 0;
	}

	Object* GetObject()
	{
		Object* pObj = NULL;
		if (_nSize == 0)
		{
			pObj = new Object();
		}
		else
		{
			pObj = _mPool.front();
			_mPool.pop_front();
			--_nSize;
		}
		return pObj;
	}

	void ReturnObject(Object* pObj)
	{
		returnObj(pObj);
		_mPool.push_back(pObj);
		++_nSize;
	}

	std::list ReturnList()
	{
		return _mPool;
	}

private:
	size_t _nSize;
	std::list<Object*> _mPool;
};

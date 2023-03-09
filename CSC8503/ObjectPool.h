#pragma once

#include <iostream>
#include <list>

template<class Object>
class ObjectPool
{
public:
	ObjectPool()
	{
		_nSize = 0;
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

	Object* GetObject2()
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

	std::list<Object*> ReturnList()
	{
		return _mPool;
	}

private:
	size_t _nSize;
	std::list<Object*> _mPool;
};

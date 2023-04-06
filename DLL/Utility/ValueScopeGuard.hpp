#pragma once

template<typename T>
class ValueScopeGuard
{
public:
	ValueScopeGuard(T& obj, const T& set, const T& unset)
		: obj(obj)
		, set(set)
		, unset(unset)
	{
		this->acquire();
	}

	~ValueScopeGuard()
	{
		this->release();
	}

	void acquire()
	{
		this->active = true;
		this->obj = this->set;
	}

	void release()
	{
		if (this->active)
		{
			this->obj = unset;
			this->active = false;
		}
	}

	bool locked() const
	{
		return this->active;
	}

	operator bool() const
	{
		return this->active;
	}

private:
	T& obj;
	T set, unset;
	bool active = false;
};

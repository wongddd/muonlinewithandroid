//////////////////////////////////////////////////////////////////////////
//  - �̱��� -
//  
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __SINGLETON_H__
#define __SINGLETON_H__

/*+++++++++++++++++++++++++++++++++++++
	CLASS.
+++++++++++++++++++++++++++++++++++++*/
template <typename T>
class Singleton
{
	static T* _Singleton;
public:
	Singleton(void)
	{
		if (_Singleton == 0)
		{
			intptr_t offset = (intptr_t)(T*)1 - (intptr_t)(Singleton <T>*)(T*)1;
			_Singleton = (T*)((intptr_t)this + offset);
		}
	}

	virtual ~Singleton(void)
	{
		_Singleton = 0;
	}

	static T& GetSingleton(void)
	{
		return (*_Singleton);
	}

	static T* GetSingletonPtr(void)
	{
		return (_Singleton);
	}

	static bool IsInitialized(void)
	{
		return _Singleton ? true : false;
	}
};

template <typename T> T* Singleton <T>::_Singleton = 0;

#endif
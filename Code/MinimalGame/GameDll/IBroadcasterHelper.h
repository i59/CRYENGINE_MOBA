#pragma once

template <class T>
struct IBroadcasterHelper
{
	void AddListener(T *pListener) { stl::push_back_unique(m_listeners, pListener); }
	void RemoveListener(T *pListener) { stl::find_and_erase(m_listeners, pListener); }

protected:
	std::vector<T *> m_listeners;
};

#define BROADCASTER_NOTIFY_LISTENERS(func) \
if(m_listeners.size() > 0) \
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) \
		(*it)->func();

#define BROADCASTER_NOTIFY_LISTENERS_1(func, param1) \
if(m_listeners.size() > 0) \
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) \
		(*it)->func(param1);

#define BROADCASTER_NOTIFY_LISTENERS_2(func, param1, param2) \
if(m_listeners.size() > 0) \
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) \
		(*it)->func(param1, param2);

#define BROADCASTER_NOTIFY_LISTENERS_3(func, param1, param2, param3) \
if(m_listeners.size() > 0) \
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) \
		(*it)->func(param1, param2, param3);

#define BROADCASTER_NOTIFY_LISTENERS_4(func, param1, param2, param3, param4) \
if(m_listeners.size() > 0) \
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) \
		(*it)->func(param1, param2, param3, param4);
#pragma once

#ifndef FUNC_H
#define FUNC_H

#include "stdafx.h"

const std::string wrkcVersion = "7.120.10.25";

#define LOG_SAVE BOOST_LOG_SEV(boost::log::trivial::logger::get(), boost::log::trivial::severity_level::trace)	\
	<< "<" << rLogger::PathToFilename(__FILE__) << ":" << __LINE__ << "> " BOOST_CURRENT_FUNCTION << " | " 	\
	<< boost::log::add_value("Line", __LINE__)

struct active_object
{
    template <typename FN> active_object(FN fn) : thread([this, fn] { while (alive) fn(); }) {}

    ~active_object() { alive = false; thread.join(); }

    active_object(const active_object&) = delete;
    active_object(active_object&&) = delete;
    active_object& operator= (active_object) = delete;


    std::atomic<bool> alive{ true };
    std::thread thread;
};

template <typename T, INT_PTR(T::* P)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)>
INT_PTR DialogBoxThis(T* pThis, HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent)
{
	return ::DialogBoxParam(hInstance, lpTemplateName, hWndParent, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> INT_PTR {
		if (uMsg == WM_INITDIALOG) SetWindowLongPtr(hWnd, DWLP_USER, lParam);
		T* pThis = reinterpret_cast<T*>(GetWindowLongPtr(hWnd, DWLP_USER));
		return pThis ? (pThis->*P)(hWnd, uMsg, wParam, lParam) : FALSE;
		}, reinterpret_cast<LPARAM>(pThis));
}

bool ProcessCheckOnRFA(HWND hwndRFA, std::wstring windowTitle);

std::string return_current_time_and_date();
std::string uudecode(const std::string& input);

int WinLMessagePump();
LRESULT LogWindowsUnlockProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

wchar_t* ReadInfString(const wchar_t* sectionName, const wchar_t* keyName);
std::string LoadConfig();
void ltrim(std::wstring& s);
inline void rtrim(std::string& s);

namespace rLogger
{
	void InitLogging();
	void LogFormatter(logging::record_view const& rec, logging::formatting_ostream& strm);
	std::string PathToFilename(std::string path);
};
#endif
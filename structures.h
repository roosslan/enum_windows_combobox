#pragma once

const unsigned short rvt_instances = 100;

typedef struct
{
	const char* name;
	const char* Klass;
	HWND handles[rvt_instances];
	int handlesFound;
	std::wstring choosenText;
	bool is_child;
	bool is_cb_visible;
	std::wstring nameBeginsW; /* window name: Autodesk Revit 2023.[*](wildcard) */
	int controlID;
	DWORD dwStyle;
	std::wstring windowsTitle;
} SearchWindowInfo;
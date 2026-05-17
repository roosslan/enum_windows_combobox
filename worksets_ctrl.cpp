#define FAMILY_PICTURE 601
#define WORKSET_PICTURE 602

#include "stdafx.h"
#include "resource.h"
#include "func.h"
#include "CWorkset.h"
#include "structures.h"

#pragma comment(lib, "WtsApi32.lib")

std::string windowCaptions = "";		/* Список заголовков окон, которые мы будем искать Revit 22, Revit 23 etc */
wchar_t* worksetBeginsWith;				/* = L"02";		в куче, не на стеке	*/
bool containsRFA;

auto wrks_ts = std::chrono::system_clock::now() + std::chrono::seconds(1);
auto wrks_fiveseconds = std::chrono::system_clock::now() + std::chrono::seconds(5);
auto rfa_ts = std::chrono::system_clock::now() + std::chrono::seconds(1);
auto rfa_fiveseconds = std::chrono::system_clock::now() + std::chrono::seconds(5);

std::wstring GetWarnWindowExecutable(int pict) {
	std::string roamingDirectory;

	char* appdata = nullptr;
	size_t sz = 0;
	if (_dupenv_s(&appdata, &sz, "APPDATA") == 0 && appdata != nullptr)
	{
		/* Convert the Windows path type to a C++ path */
		roamingDirectory = appdata;
		free(appdata);
	}

	std::wstring appDataDirectory(roamingDirectory.begin(), roamingDirectory.end());

	if (pict == FAMILY_PICTURE)
		return appDataDirectory + L"\\rfaWindow.exe";
	else
		return appDataDirectory + L"\\warnWindow.exe";
};

BOOL CALLBACK wrkstEnumWindowsProc(HWND hwnd, LPARAM lParam)
try {	
	/* WINDOWINFO* windowInfo = (WINDOWINFO*)lParam;				/* For Statusbar's dwStyle / WS_VISIBLE or not		*/
	SearchWindowInfo* cb_wi = (SearchWindowInfo*)lParam;
	/* cb_wi->dwStyle = windowInfo->dwStyle;						*/

	/*	if (wi->handlesFound == 100)
		/* Если все проверки прошли, сохраняем текущий хэндл окна в передаваемой структуре и возвращаем FALSE
		return FALSE;
	*/	
	int rc;
	const int size = 1024; /* Длина заголовка окна в символах */
	BOOL res;	
	/* Тут идут проверки нужных свойств, если они не проходят, надо вернуть TRUE		*/

	wchar_t name[size];
	memset(name, 0, size);
	char class_buf[256];

	/* находим класс и имя окна	*/

	if (cb_wi->Klass) {
		rc = GetClassNameA(hwnd, class_buf, sizeof(class_buf) - 1);
	}

	if (cb_wi->name) {
		res = GetWindowTextW(hwnd, name, size);
		std::wstring windName(name);

		try
		{
			if (containsRFA)
			{
				/* 1s timer	*/
				if (rfa_ts <= std::chrono::system_clock::now()) {

					/* start func every 5s */
					if (rfa_fiveseconds <= std::chrono::system_clock::now()) {
						{
							containsRFA = false;
							ShellExecute(nullptr, L"open", GetWarnWindowExecutable(FAMILY_PICTURE).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
						}

						LOG_SAVE << "cb_wi->choosenText values: " << cb_wi->choosenText;

						rfa_fiveseconds = std::chrono::system_clock::now() + std::chrono::seconds(20);
					}

					rfa_ts += std::chrono::seconds(1);
				}
			}
		}
		catch (...)
		{
			if (!hwnd)
				LOG_SAVE << "HWND is nullptr";
		}

		/* Do we need an additional check? -				*/
		/* Statusbar's Id is always 0xE801 = dec 59393		*/
		if (cb_wi->is_child)
		{
			int theID = GetDlgCtrlID(hwnd);
			/* Worksets combobox's control ID is always:
				Revit 2023:   0x6d0 = dec 1744	
				Revit 2023.1: 0x6d2 = dec 1746	*/
			if (theID == 1744 || theID == 1746)
			{				
				cb_wi->dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
				/* 40000213 = 1073742355	*/
				/* 50000213 = 1342177811 -> WS_VISIBLE	*/
				if (cb_wi->dwStyle & WS_VISIBLE)
				{
					cb_wi->is_cb_visible = true;

					try
					{
//						wchar_t choosenText[MAXCHAR];	/* Убрали TCHAR из кучи в стек */
						LPTSTR choosenText = new TCHAR[ComboBox_GetLBTextLen(hwnd, ComboBox_GetCurSel(hwnd))];
						ComboBox_GetLBText(hwnd, ComboBox_GetCurSel(hwnd), choosenText);

						cb_wi->choosenText = choosenText;

						ltrim(cb_wi->choosenText);
						const wchar_t* combobsSelectedLine = worksetBeginsWith;
						
						if (ComboBox_FindString(hwnd, 0, combobsSelectedLine) != -1)		/* В списке рабочих наборов присутствует начинающийся на 02 */
							if (!ComboBox_GetDroppedState(hwnd))							/* Обрабатываем  ложное срабатывание. Если комбобокс раскрыт, то ничего не делаем */
								if (cb_wi->choosenText.rfind(combobsSelectedLine, 0) != 0)	/* pos=0 limits the search to the prefix	*/
								/* selText starts with prefix	*/
										/* 1s timer	*/
										if (wrks_ts <= std::chrono::system_clock::now()) {

											/* start func every 5s */
											if (wrks_fiveseconds <= std::chrono::system_clock::now()) {
												wrks_fiveseconds = std::chrono::system_clock::now() + std::chrono::seconds(20);												
												ShellExecute(nullptr, L"open", GetWarnWindowExecutable(WORKSET_PICTURE).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
												
												LOG_SAVE << "cb_wi->choosenText values: " << cb_wi->choosenText;
												wrks_fiveseconds = std::chrono::system_clock::now() + std::chrono::seconds(20);
											}
											wrks_ts += std::chrono::seconds(1);
										}
					}
					catch (...)
					{
						if (!hwnd)
							LOG_SAVE << "HWND is nullptr";
					}					
				}
				else {
					cb_wi->is_cb_visible = false;
				}
			}
			return TRUE;
		}

		if (!cb_wi->nameBeginsW.empty())
		{
			if (windName.rfind(cb_wi->nameBeginsW.c_str(), 0) == 0)
			{
				rc = GetClassNameA(hwnd, class_buf, sizeof(class_buf) - 1);
				cb_wi->windowsTitle = name;
				cb_wi->handles[cb_wi->handlesFound++] = hwnd;

				/* Проверяем заголовок на наличие ".rfa" */
				if (ProcessCheckOnRFA(cb_wi->handles[cb_wi->handlesFound - 1], cb_wi->windowsTitle)){
					containsRFA = true;
				}
				return TRUE;
			}
		}
	}
	return TRUE;	
}
catch (const std::exception e)
{
	LOG_SAVE << e.what();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
/* int main(int argc, char* argv[])	*/
{
	rLogger::InitLogging();
	CWorkset Workset;
	CMSSQLServer msdb("");

	LOG_SAVE << "Create infinite loops" << std::endl;	

	/* Фоновой loop для записи времени нажатия Win+L */
	active_object CatchLoginIntoWindows([&Workset] {
		WinLMessagePump();
	});

	active_object isWorksetControlOn([&Workset, &msdb] {
		Workset.repeatCheck(&msdb);
	});

	windowCaptions = LoadConfig();
	LOG_SAVE << "windowCaptions: " << windowCaptions << std::endl;

	LOG_SAVE << "Read workset name beginning with... " << std::endl;
	const wchar_t* stackWorksetBeginsWith = ReadInfString(L"RVT", L"Workset");
	size_t stackWorksetBeginsW_Len = wcslen(stackWorksetBeginsWith) + 1;

	/* allocate heap memory, cause on stack it will be destroyed	*/
	worksetBeginsWith = new wchar_t[stackWorksetBeginsW_Len + 1];

	/* copy the string to heap memory */
	wcscpy_s(worksetBeginsWith, stackWorksetBeginsW_Len, stackWorksetBeginsWith);

	LOG_SAVE << "stackWorksetBeginsWith: " << worksetBeginsWith << std::endl;
	
	clock_t start = clock();
	/* 2 seconds sleep alternative */
	while (true) {
		clock_t current = clock();
		double elapsed = double(current - start) / CLOCKS_PER_SEC;
		if (elapsed >= 2) {
			break;
		}
	}
	/* std::this_thread::sleep_for(std::chrono::seconds(2)); */
/*
	rfa_ts = std::chrono::system_clock::now() + std::chrono::seconds(1);
	rfa_fiveseconds = std::chrono::system_clock::now() + std::chrono::seconds(5);
	wrks_ts = std::chrono::system_clock::now() + std::chrono::seconds(1);
	wrks_fiveseconds = std::chrono::system_clock::now() + std::chrono::seconds(5);
	*/
	while (true)
	{
		// Получаем текущее значение из БД
		int isActive = Workset.gid; /* 1 - программа работает, 0 - приостановлена */

		bool isAffectedAccount = true;

		std::vector <std::string> list_string;
		list_string.reserve(Workset.domain_accounts.size());

		//for (const auto& wide : Workset.domain_accounts) {
		for (int i = 0; i < Workset.domain_accounts.size(); ++i) {			
			try {				
					list_string.push_back(Workset.domain_accounts[i]);
			}
			catch (const std::length_error& e) {
				/* ... */
			}
		}

		for (auto& domain_name : list_string) {
			size_t len = wcstombs(nullptr, Workset.domain_account.c_str(), 0) + 1;
			char* buffer = new char[len];
			wcstombs(buffer, Workset.domain_account.c_str(), len);
			std::string str(buffer);
			delete[] buffer;

			std::transform(domain_name.begin(), domain_name.end(), domain_name.begin(), [](wchar_t c) {
				return std::tolower(c);
			});

			if (str == domain_name) {
				isAffectedAccount = false;
				break;
			}
		}	

		if (isActive == 1 && isAffectedAccount) // || name in account);
		{
			SearchWindowInfo wi;
			wi.name = "WindowName";
			wi.Klass = "ClassName";
			wi.handlesFound = 0;

			// To loop over Revit's child windows
			SearchWindowInfo wi_kid(wi);
			wi_kid.is_child = true;
			wi_kid.is_cb_visible = false;

			wi.is_child = false;
			wi.nameBeginsW = L"Autodesk Revit 202";	/* = windowCaptions? */
			try
			{
				EnumWindows(&wrkstEnumWindowsProc, (LPARAM)(&wi));

				for (int i = 0; i < wi.handlesFound; i++) {
					wi_kid.handles[i] = wi.handles[i];
				}

				for (int i = 0; i < wi.handlesFound; i++)
				{
					/* Дальше ищем дочернее окно аналогично, но с помощью */
					EnumChildWindows(wi.handles[i], &wrkstEnumWindowsProc, (LPARAM)(&wi_kid));
				}
			}
			catch (const std::exception e) {
				LOG_SAVE << e.what();
			}
		}
		else
		{
			/* Логика приостановки программы. Например, можно просто ждать или выполнять другие действия */
			/* Задержка, чтобы не перегружать процессор */
			clock_t start = clock();
			/* 2 seconds sleep alternative */
			while (true) {
				clock_t current = clock();
				double elapsed = double(current - start) / CLOCKS_PER_SEC;
				if (elapsed >= 2) {
					break;
				}
			}
			/* std::this_thread::sleep_for(std::chrono::seconds(1)); */
		}
	}
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0); /* Posts a WM_QUIT message to the thread's message queue */
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps); /* Prepares the specified window for painting */
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1)); /* Fills the client area with the background brush */
		EndPaint(hwnd, &ps);   /* Releases the display device context */
	}
	return 0;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam); // Default message processing
	}
}
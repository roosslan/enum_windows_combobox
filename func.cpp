
#include "func.h"

std::string return_current_time_and_date(){
	auto now = std::chrono::system_clock::now();
	time_t in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%d-%m-%Y %X");
	return ss.str();
}

bool ProcessCheckOnRFA(HWND hwndRFA, std::wstring windowTitle){
	std::string strWindowTitle(windowTitle.begin(), windowTitle.end());

	const std::string lowerWindowTitle = boost::algorithm::to_lower_copy(strWindowTitle);

	if (lowerWindowTitle.contains(".rfa"))
	{
		return true;
	};
	return false;
}

LRESULT LogWindowsUnlockProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if (uMsg == WM_WTSSESSION_CHANGE)
	{
		switch (wParam)
		{
		case WTS_SESSION_LOCK:
			/* std::cout << "Session locked (Win+L pressed)" << std::endl;	*/
			break;

		case WTS_SESSION_UNLOCK:

			SQLHDBC dbc;
			SQLHSTMT stmt;
			SQLHENV env;

			// Подготовка подключения к БД
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
			SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

			// Подключаемся к серверу
			std::string passwd = uudecode("");
			std::string connectionStr = "DRIVER={ODBC Driver 17 for SQL Server}; Encrypt=No; TrustServerCertificate=Yes; Trusted_Connection=No; SERVER=sql_server_hostname; DATABASE=my_database_name; UID=user_login; PWD=" + passwd;
			const SQLCHAR* connStrChar = reinterpret_cast<const unsigned char*>(connectionStr.c_str());

			SQLRETURN retcode = SQLDriverConnectA(dbc, NULL, const_cast<SQLCHAR*>(connStrChar), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

				SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

				TCHAR name[UNLEN + 1];
				DWORD size = UNLEN + 1;

				(GetUserName(name, &size));
				std::wstring user_name = name;

				// Преобразуем имя пользователя в нижний регистр
				std::transform(user_name.begin(), user_name.end(), user_name.begin(), [](wchar_t c) {
					return std::tolower(c);
					});

				std::string domain_account(user_name.begin(), user_name.end());

				std::string query = "INSERT into el_win (id, account, datetime) VALUES((SELECT ISNULL(MAX(id) + 1, 0) FROM el_win), '" + domain_account + "', '" + return_current_time_and_date() + "');";
				SQLCHAR* squery = reinterpret_cast<SQLCHAR*>(const_cast<char*>(query.c_str()));

				if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, squery, SQL_NTS))) {
					std::cerr << "Error executing query" << std::endl;
				}

				// Освобождаем statement
				SQLFreeHandle(SQL_HANDLE_STMT, stmt);

				SQLDisconnect(dbc);
				break;
			}
			else LOG_SAVE << "Cannot write WTS_SESSION_ event into DB. retcode = " + retcode;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WinLMessagePump(){
	/* Создаем скрытое окно, чтобы получать сообщения о нажатии WIN+L */
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = LogWindowsUnlockProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"SessionMonitorClass";
	RegisterClass(&wc);
	HWND hwnd = CreateWindowEx(0, L"SessionMonitorClass", L"", 0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
	/* Register for session notifications */
	WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);

	/* Message loop */
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

wchar_t* ReadInfString(const wchar_t* sectionName, const wchar_t* keyName){
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

	std::wstring infFile = appDataDirectory + L"\\worksets_ctrl.ini";

	wchar_t wc_Val[_MAX_FNAME] = L"";
	GetPrivateProfileStringW(sectionName, keyName, nullptr, wc_Val, std::size(wc_Val),	infFile.c_str());
	return wc_Val;
}

/* trim from start (in place) */
void ltrim(std::wstring& s){
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

/* trim from end (in place) */
inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

std::string rLogger::PathToFilename(std::string path) {
	return path.substr(path.find_last_of("/\\") + 1);
}

void rLogger::LogFormatter(logging::record_view const& rec, logging::formatting_ostream& strm){
	// strm << logging::extract< int >("Line", rec) << ":";
	strm << rec[expr::smessage];
}

void rLogger::InitLogging(){
	std::string roamingDirectory;

	char* appdata = nullptr;
	size_t sz = 0;
	if (_dupenv_s(&appdata, &sz, "APPDATA") == 0 && appdata != nullptr)
	{
		/* Convert the Windows path type to a C++ path */
		roamingDirectory = appdata;
		free(appdata);
	}

	boost::log::add_common_attributes();

	auto consoleSink = boost::log::add_console_log(std::clog);
	consoleSink->set_formatter(&LogFormatter);
	logging::core::get()->add_sink(consoleSink);

	auto fsSink = boost::log::add_file_log(
		boost::log::keywords::file_name = roamingDirectory + "\\worksets_ctrl_%d.%m.%Y-%H_%M_%S.log",
		keywords::format = "%TimeStamp% % Message % ",
		boost::log::keywords::rotation_size = 10 * 1024 * 1024,
		boost::log::keywords::min_free_space = 30 * 1024 * 1024,
		boost::log::keywords::open_mode = std::ios_base::app);

	fsSink->locked_backend()->auto_flush(true);

	LOG_SAVE << "Worksets controller v" << wrkcVersion << "'s rLogger init " << std::endl;
}

std::string LoadConfig(){
	std::string ret = "";

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

	std::wstring infFilePath = appDataDirectory + L"\\worksets_ctrl.ini";

	std::ifstream inFile;
	inFile.open(infFilePath);

	std::stringstream strStream;
	strStream << inFile.rdbuf();				 //read the file
	const std::string infFile = strStream.str(); // the content of the file

	/* Захватываем содержимое секции [Strings]: */
	const std::regex re(R"(\[Strings\]((?:\n\s*[^\]\[\s].*)+))");

	std::string sectionContent = "";
	std::smatch matches;
	if (std::regex_search(infFile, matches, re))
	{
		auto matchIterator = matches.begin();
		/* Берём совпадение без имени секции: */
		++matchIterator;
		ret = *matchIterator;
	}
	inFile.close();
	return ret;
}

std::string uudecode(const std::string& input) {
	std::string decoded;
	int len = input.length();

	for (int i = 0; i < len; i += 4) {
		unsigned int combined = 0;
		for (int j = 0; j < 4; ++j) {
			unsigned char c = input[i + j];
			c = (c == '`') ? 0 : (c - 32) & 0x3F;  // decode character
			combined = (combined << 6) | c;
		}

		decoded += (combined >> 16) & 0xFF;
		if (i + 2 < len) decoded += (combined >> 8) & 0xFF;
		if (i + 3 < len) decoded += combined & 0xFF;
	}

	return decoded;
}

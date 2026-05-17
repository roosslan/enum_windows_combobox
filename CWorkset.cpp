#include "func.h"
#include "CWorkset.h"

CWorkset::CWorkset(){
	TCHAR name[UNLEN + 1];
	DWORD size = UNLEN + 1;

	(GetUserName(name, &size));
	std::wstring user_name(name);

	// Преобразуем имя пользователя в нижний регистр
	std::transform(user_name.begin(), user_name.end(), user_name.begin(), [](wchar_t c) {
		return std::tolower(c);
	});

	domain_account = user_name;
}

int CWorkset::repeatCheck(CMSSQLServer* msdb){
	auto data = fetchData(msdb);

    /* Выводим полученные данные */
	int id;
	int time_;
	std::tie(id, time_) = data;

	gid = id;
	clock_t start = clock();
	/* 180 seconds sleep alternative */
	while (true){
		clock_t current = clock();
		double elapsed = double(current - start) / CLOCKS_PER_SEC;
		if (elapsed >= time_) {
			break;
		}
	}

	/* std::this_thread::sleep_for(std::chrono::seconds(time_)); */
	return 0;
}

std::tuple<int, int> CWorkset::fetchData(CMSSQLServer* msdb){
    int time = msdb->SELECT("SELECT value FROM settings WHERE name = 'WorksetControlRepeatSeconds'");
    int isOn = msdb->SELECT("SELECT value FROM settings WHERE name = 'WorksetControlOn'");
	/* isOn = 1; /* Debug-mode */

    SQLCHAR* query = (SQLCHAR*)"SELECT domain_account FROM wrksIgnoreAccounts";
    domain_accounts = msdb->SELECT(query);

    // Возвращаем кортеж
    return std::make_tuple(isOn, time);
}
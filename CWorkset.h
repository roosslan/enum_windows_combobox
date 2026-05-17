#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "stdafx.h"
#include "CMSSQLServer.h"

class CWorkset
{
public:
	/* Логин пользователя, который вошел в систему */
	std::wstring domain_account;

	std::vector<std::string> domain_accounts;

	//  переменная для хранения значения из БД
	int dbValue = 0; /* Инициализируем значением 1 (работает) */

	explicit CWorkset();
	//~CWorkset();

	int gid = 0;

	std::tuple<int, int> fetchData(CMSSQLServer* msdb);
	
	int repeatCheck(CMSSQLServer* msdb);
};
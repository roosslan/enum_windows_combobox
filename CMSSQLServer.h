#pragma once

#include "stdafx.h"
#include "func.h"

class CMSSQLServer
{
	SQLHDBC dbc;
public:

	CMSSQLServer(std::string UUE_passwd);

	~CMSSQLServer();
	std::vector<std::string> SELECT(SQLCHAR* query);
	int SELECT(std::string query);
	void INSERT(std::string domain_account);
};


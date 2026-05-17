#include "CMSSQLServer.h"

CMSSQLServer::CMSSQLServer(std::string UUE_passwd){
	SQLHENV env;

	// Подготовка подключения к БД
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

	// Подключаемся к серверу
	std::string passwd = uudecode("");
	std::string connectionStr = "DRIVER={ODBC Driver 17 for SQL Server}; Encrypt=No; TrustServerCertificate=Yes; Trusted_Connection=No; SERVER=sql_server_hostname; DATABASE=my_database_name; UID=user_login; PWD=" + passwd;
	const SQLCHAR* connStrChar = reinterpret_cast<const unsigned char*>(connectionStr.c_str());

	auto ret = SQLDriverConnectA(dbc, NULL, const_cast<SQLCHAR*>(connStrChar), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

}

CMSSQLServer::~CMSSQLServer(){
	SQLDisconnect(dbc);
}

std::vector<std::string> CMSSQLServer::SELECT(SQLCHAR* query){
	SQLHSTMT stmt;
	std::vector<std::string> v_rret;
	
	// Создаем statement для выполнения второго запроса
	SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

	// Запрос для получения domain_account
	if (SQL_SUCCEEDED(SQLExecDirectA(stmt, query, SQL_NTS))) {
		/* Буфер для хранения domain_account */
		SQLCHAR accountBuffer[50];  
		SQLBindCol(stmt, 1, SQL_C_CHAR, accountBuffer, sizeof(accountBuffer), NULL);

		/* Извлекаем данные */
		while (SQLFetch(stmt) == SQL_SUCCESS) {
			/* Добавляем извлеченную строку в список */
			v_rret.push_back(reinterpret_cast<char*>(accountBuffer));
		}
	}
	else {
		std::cerr << "Error executing query" << std::endl;
	}

	// Освобождаем statement
	SQLFreeHandle(SQL_HANDLE_STMT, stmt);

	return v_rret;
}

int CMSSQLServer::SELECT(std::string query){
	SQLHSTMT stmt;

	// Создаем statement для выполнения первого запроса
	SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

	SQLCHAR* squery = reinterpret_cast<SQLCHAR*>(const_cast<char*>(query.c_str()));
	
	int time = 0;

	if (SQL_SUCCEEDED(SQLExecDirectA(stmt, squery, SQL_NTS))) {
		/* Привязываем колонки к переменным */
		SQLBindCol(stmt, 1, SQL_C_LONG, &time, 0, NULL);

		/* Извлекаем данные */
		if (SQLFetch(stmt) == SQL_SUCCESS) {
			// Значения id и time извлечены
		}
	}
	else {
		std::cerr << "Error executing query" << std::endl;
	}

	/* Освобождаем statement */
	SQLFreeHandle(SQL_HANDLE_STMT, stmt);
	return time;
}

void CMSSQLServer::INSERT(std::string domain_account){
	SQLHSTMT stmt;

	// Создаем statement для выполнения первого запроса
	SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

	std::string query = "INSERT into settings (id, account, datetime) VALUES(0, " + domain_account + ", " + return_current_time_and_date() + ");" ;
	SQLCHAR* squery = reinterpret_cast<SQLCHAR*>(const_cast<char*>(query.c_str()));

	
	if (!SQL_SUCCEEDED(SQLExecDirectA(stmt, squery, SQL_NTS))) {
		std::cerr << "Error executing query" << std::endl;
	}

	// Освобождаем statement
	SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

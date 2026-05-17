#pragma once
#define _WIN32_WINNT     0x0A00

#include <afxwin.h>         // основные и стандартные компоненты MFC
#include <afxext.h>         // Расширения MFC
#include "afxdialogex.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <format>
#include <chrono>
#include <thread>
#include <stdexcept>

#include <windows.h>
#include "winuser.h"
#include <windowsx.h>
#include <WtsApi32.h>

#include <Lmcons.h>
#include <vector>
#include <algorithm>
#include <codecvt>
#include <tuple>
#include <list>
#include <cctype>
#include <shlobj.h>
#include <regex>

#include "sqlext.h"
#include "sqltypes.h"
#include "sql.h"



#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/current_function.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace src = boost::log::sources;
namespace logging = boost::log;
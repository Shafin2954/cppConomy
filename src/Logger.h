#pragma once

#include <iostream>
#include <string>

#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_WARNING(msg) std::cout << "[WARN] " << msg << std::endl
#define LOG_ALERT(msg) std::cout << "[ALERT] " << msg << std::endl
#define LOG_SUCCESS(msg) std::cout << "[OK] " << msg << std::endl

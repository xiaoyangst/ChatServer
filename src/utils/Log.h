/**
  ******************************************************************************
  * @file           : Log.h
  * @author         : xy
  * @brief          : Safe and enhanced log utility with log4cpp
  * @date           : 2025/4/17
  ******************************************************************************
  */

#ifndef CHAT_SRC_UTILS_LOG_H_
#define CHAT_SRC_UTILS_LOG_H_

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <cstdarg>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>

class Log : public std::enable_shared_from_this<Log> {
 public:
  static Log *getInstance() {
	  static Log instance;
	  return &instance;
  }

  void setLogLevel(log4cpp::Priority::Value priority) {
	  m_rootCategory.setPriority(priority);
  }

  void setLogFilePaths(const std::string &errorPath, const std::string &rollPath) {
	  m_errorPath = errorPath;
	  m_rollPath = rollPath;
	  reinitAppenders();
  }

  void logMessage(log4cpp::Priority::PriorityLevel level,
				  const char *file, const char *func, int line,
				  const char *format, va_list args) {
	  char messageBuffer[1024];
	  vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);

	  std::ostringstream oss;
	  oss << messageBuffer << " {file:" << file << " func:" << func << " line:" << line << "}";

	  switch (level) {
		  case log4cpp::Priority::FATAL: m_rootCategory.fatal(oss.str()); break;
		  case log4cpp::Priority::ERROR: m_rootCategory.error(oss.str()); break;
		  case log4cpp::Priority::WARN:  m_rootCategory.warn(oss.str()); break;
		  case log4cpp::Priority::INFO:  m_rootCategory.info(oss.str()); break;
		  case log4cpp::Priority::DEBUG: m_rootCategory.debug(oss.str()); break;
		  default: m_rootCategory.info(oss.str()); break;
	  }
  }

  void fatal(const char *file, const char *func, int line, const char *format, ...) {
	  va_list args; va_start(args, format);
	  logMessage(log4cpp::Priority::FATAL, file, func, line, format, args);
	  va_end(args);
  }

  void error(const char *file, const char *func, int line, const char *format, ...) {
	  va_list args; va_start(args, format);
	  logMessage(log4cpp::Priority::ERROR, file, func, line, format, args);
	  va_end(args);
  }

  void warn(const char *file, const char *func, int line, const char *format, ...) {
	  va_list args; va_start(args, format);
	  logMessage(log4cpp::Priority::WARN, file, func, line, format, args);
	  va_end(args);
  }

  void info(const char *file, const char *func, int line, const char *format, ...) {
	  va_list args; va_start(args, format);
	  logMessage(log4cpp::Priority::INFO, file, func, line, format, args);
	  va_end(args);
  }

  void debug(const char *file, const char *func, int line, const char *format, ...) {
	  va_list args; va_start(args, format);
	  logMessage(log4cpp::Priority::DEBUG, file, func, line, format, args);
	  va_end(args);
  }

  ~Log() {
	  log4cpp::Category::shutdown();
  }

  // Delete copy/move constructors
  Log(const Log &) = delete;
  Log &operator=(const Log &) = delete;
  Log(Log &&) = delete;
  Log &operator=(Log &&) = delete;

 private:
  Log() : m_rootCategory(log4cpp::Category::getRoot().getInstance("rootCategory")),
	  m_errorPath("log/error.log"),
	  m_rollPath("log/roll.log") {
	  reinitAppenders();
	  setLogLevel(log4cpp::Priority::INFO);
  }

  void reinitAppenders() {
	  m_rootCategory.removeAllAppenders();
	  m_appenders.clear();
	  m_layouts.clear();

	  // Console appender
	  auto layoutConsole = std::make_shared<log4cpp::PatternLayout>();
	  layoutConsole->setConversionPattern("%d [%p] %m%n");

	  auto consoleAppender = std::make_shared<log4cpp::OstreamAppender>("console", &std::cout);
	  consoleAppender->setLayout(layoutConsole.get());

	  m_rootCategory.addAppender(consoleAppender.get());

	  m_appenders.push_back(consoleAppender);
	  m_layouts.push_back(layoutConsole);

	  // Error file appender
	  auto layoutError = std::make_shared<log4cpp::PatternLayout>();
	  layoutError->setConversionPattern("%d [%p] %m%n");

	  auto fileAppender = std::make_shared<log4cpp::FileAppender>("fileAppender", m_errorPath);
	  fileAppender->setLayout(layoutError.get());

	  m_rootCategory.addAppender(fileAppender.get());

	  m_appenders.push_back(fileAppender);
	  m_layouts.push_back(layoutError);

	  // Rolling appender
	  auto layoutRolling = std::make_shared<log4cpp::PatternLayout>();
	  layoutRolling->setConversionPattern("%d [%p] %m%n");

	  auto rollingAppender = std::make_shared<log4cpp::RollingFileAppender>(
		  "rollingFileAppender", m_rollPath, 5 * 1024 * 1024, 5);

	  rollingAppender->setLayout(layoutRolling.get());

	  m_rootCategory.addAppender(rollingAppender.get());

	  m_appenders.push_back(rollingAppender);
	  m_layouts.push_back(layoutRolling);
  }

 private:
  log4cpp::Category &m_rootCategory;
  std::string m_errorPath;
  std::string m_rollPath;

  std::vector<std::shared_ptr<log4cpp::Appender>> m_appenders;
  std::vector<std::shared_ptr<log4cpp::Layout>> m_layouts;
};

// Logging macros
#define LOG_FATAL(fmt, ...) Log::getInstance()->fatal(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Log::getInstance()->error(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Log::getInstance()->warn(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Log::getInstance()->info(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Log::getInstance()->debug(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_SET_LEVEL(level) Log::getInstance()->setLogLevel(level)

#endif  // CHAT_SRC_UTILS_LOG_H_

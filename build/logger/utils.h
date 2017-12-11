#pragma once

#include <string>
#include <list>
#include <memory>
#include <thread>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#else
#include <io.h>
#include <windows.h>
#include <direct.h>
#endif

class LogUtils
{
public:
	LogUtils(const uint64_t &maxLogSize, const std::string &logPtah);
	~LogUtils();

public:
	static std::string GetAppName();//获取进程名字
	static std::string GetAppPath();//获取程序所在目录,结尾无斜杠

	static int ReadLogSeverity(const std::string &cfgPath);//根据配置文件读取日志级别
	static int ReadLogMaxSize(const std::string &cfgPath);//根据配置文件读取文件夹最大日志大小

	static std::string CreateLogPath(const std::string &logPath);//根据日志路径创建日志路径,当路径为空时,默认为app_path\logs

	static std::string GetLogdayPrev(const std::string &logFile);
	static std::string GetLogdayNow();

private:
	void LogThread();

private:
	void RecycleLog(const std::string &logPath);//回收日志
	static bool CreateFullPath(const std::string &fullPath);//根据完整路径创建完整目录
	static void TrySleep(int ms);//休眠毫秒

private:
	uint64_t _maxLogSize;//最大日志大小
	std::string _logPath;//日志路径

	std::shared_ptr<std::thread> _thread;
	bool _stop;
};

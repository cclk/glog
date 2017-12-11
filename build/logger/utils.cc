#include "utils.h"
#include "logger.h"
#include "glog/logging.h"

#ifdef _WIN32
#include "port.h"
#endif

typedef struct fileInfo_
{
	char path[1024];
	unsigned long long size;
	unsigned long long modifyTime;

	fileInfo_() : size(0), modifyTime(0) { }

	bool operator < (fileInfo_ &b)
	{
		return modifyTime < b.modifyTime;
	}
}fileInfo;

//////////////////////////////////////////////////////////////////////////
LogUtils::LogUtils(const uint64_t &maxLogSize, const std::string &logPtah)
	: _maxLogSize(maxLogSize)
	, _logPath(logPtah)
	, _thread(nullptr)
	, _stop(false)
{
	_thread = std::make_shared<std::thread>(std::bind(&LogUtils::LogThread, this));
}

LogUtils::~LogUtils()
{
	_stop = true;
	if (_thread && _thread->joinable())
	{
		_thread->join();
	}
}

std::string LogUtils::GetAppName()
{
	std::string appName;
#ifdef _WIN32
	char szAppName[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szAppName, MAX_PATH);
	appName = szAppName;
	int begin = appName.rfind('\\');
	int end = appName.rfind('.');

	if (end == appName.npos || begin == appName.npos || end <= begin)
	{
		appName = "unknow";
	}
	else
	{
		appName = appName.substr(begin + 1, end - begin - 1);
	}
#else
	char szAppName[1024] = { 0 };
	int rslt = readlink("/proc/self/exe", szAppName, 1023);
	if (rslt < 0 || (rslt >= 1023))
	{
		appName = "unknow";
	}
	else
	{
		appName = szAppName;
	}
#endif

	return appName;
}

std::string LogUtils::GetAppPath()
{
	std::string appPath;
#ifdef _WIN32
	char szAppPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szAppPath, MAX_PATH);
	(strrchr(szAppPath, '\\'))[0] = 0;		//结尾无斜杠
	//(strrchr(szAppPath, '\\'))[1] = 0;	// 结尾有斜杠
	appPath = szAppPath;
#else
	char szAppPath[1024] = { 0 };
	int rslt = readlink("/proc/self/exe", szAppPath, 1023);
	if (rslt < 0 || (rslt >= 1023))
	{
		appPath = "";
	}
	else
	{
		szAppPath[rslt] = '\0';
		for (int i = rslt; i >= 0; i--)
		{
			if (szAppPath[i] == '/')
			{
				szAppPath[i] = '\0';

				appPath = szAppPath;
				break;
			}
		}
	}
#endif

	return appPath;
}

bool LogUtils::CreateFullPath(const std::string &fullPath)
{
	int offset = 1;
	std::string subPath;

	do
	{
		int findPos = fullPath.find('\\', offset);
		if (fullPath.npos == findPos)
		{
			findPos = fullPath.find('/', offset);
		}

		if (fullPath.npos == findPos)
		{
#ifdef _WIN32
			_mkdir(fullPath.c_str());
#else
			mkdir(fullPath.c_str(), 0755);
#endif
			break;
		}

		subPath = fullPath.substr(0, findPos);
		offset = findPos + 1;

#ifdef _WIN32
		if (0 != _access(subPath.c_str(), 0)//file not exist
			&& 0 != _mkdir(subPath.c_str()))//create dir faild
		{
			fprintf(stderr, "mkdir error:%s\n", subPath.c_str());
			break;
		}
#else
		if (0 != access(subPath.c_str(), 0)//file not exist
			&& 0 != mkdir(subPath.c_str(), 0755))//create dir faild
		{
			fprintf(stderr, "mkdir error:%s\n", subPath.c_str());
			break;
		}
#endif
	} while (1);

#ifdef _WIN32
	return (0 == _access(fullPath.c_str(), 0));
#else
	return (0 == access(fullPath.c_str(), 0));
#endif
}

int LogUtils::ReadLogSeverity(const std::string &cfgPath)
{
	FILE *file = fopen(cfgPath.c_str(), "rb");
	if (file == NULL)
	{
		return -1;
	}

	fseek(file, 0, SEEK_END);
	int len = ftell(file);

	static char *buff = (char *)malloc(len);
	if (buff == NULL)
	{
		fclose(file);
		return -1;
	}

	fseek(file, 0, SEEK_SET);
	len = fread(buff, 1, len, file);
	fclose(file);
	file = NULL;

	std::string flag = "LogSeverity=";
	std::string cfg = buff;
	int pos = cfg.find(flag);
	if (cfg.npos != pos)
	{
		std::string valueStr = cfg.substr(pos + flag.length(), 1);
		return atoi(valueStr.c_str());
	}

	return -1;
}

int LogUtils::ReadLogMaxSize(const std::string &cfgPath)
{
	FILE *file = fopen(cfgPath.c_str(), "rb");
	if (file == NULL)
	{
		return -1;
	}

	fseek(file, 0, SEEK_END);
	int len = ftell(file);

	static char *buff = (char *)malloc(len);
	if (buff == NULL)
	{
		fclose(file);
		return -1;
	}

	fseek(file, 0, SEEK_SET);
	len = fread(buff, 1, len, file);
	fclose(file);
	file = NULL;

	std::string flag = "LogMaxSize=";
	std::string cfg = buff;
	int pos = cfg.find(flag);
	if (cfg.npos != pos)
	{
		std::string valueStr;
		int pos_end = cfg.find("\n", cfg.npos);
		if (pos_end == cfg.npos || pos_end < pos)
		{
			valueStr = cfg.substr(pos + flag.length());
		}
		else
		{
			valueStr = cfg.substr(pos + flag.length(), pos_end - pos - flag.length());
		}

		int logMaxSize = atoi(valueStr.c_str());
		if (logMaxSize < 0)
		{
			return -1;
		}
		else
		{
			return logMaxSize;
		}
	}

	return -1;
}

void LogUtils::TrySleep(int ms)
{
#ifdef WIN32
	Sleep(ms);
#else
	usleep(1000 * ms);
#endif
}

std::string LogUtils::CreateLogPath(const std::string &logPath)
{
	std::string path = logPath;
	if (path.empty())
	{
#ifdef _WIN32
		path = GetAppPath() + "\\logs";
#else
		path = GetAppPath() + "/logs";
#endif

		fprintf(stderr, "path:%s\n", path.c_str());
	}

	if (!CreateFullPath(path))
	{
		fprintf(stderr, "CreateFullPath error:%s\n", path.c_str());
		path.clear();
	}

	return path;
}

std::string LogUtils::GetLogdayPrev(const std::string &logFile)
{
	std::string oldDay;
	FILE *file = fopen(logFile.c_str(), "rb+");

	do
	{
		if (!file)
		{
			break;
		}

		char temp[64] = { 0 };
		if (fread(temp, 1, 60, file) < 41)
		{
			break;
		}

		std::string line = temp;
		oldDay = line.substr(21, 4) + line.substr(26, 2) + line.substr(29, 2);
	} while (0);

	if (file)
	{
		fclose(file);
		file = nullptr;
	}

	return oldDay;
}

std::string LogUtils::GetLogdayNow()
{
	time_t time_now = time(NULL);
	struct ::tm tm_now;
	localtime_r(&time_now, &tm_now);

	char temp[64] = { 0 };
	sprintf(temp, "%04d%02d%02d", 1900 + tm_now.tm_year, tm_now.tm_mon + 1, tm_now.tm_mday);
	return std::string(temp);
}

void LogUtils::LogThread()
{
	std::string logIniPath;
	int logSeverity = google::GLOG_INFO;
	int logMaxSize = _maxLogSize;

#ifdef _WIN32
	logIniPath = GetAppPath() + "\\glog.ini";
#else
	logIniPath = GetAppPath() + "/glog.ini";
#endif

	while (!_stop)
	{
		static int index = 0;
		++index;
		index %= 1000;

		//sleep
		if (0 != index)
		{
			TrySleep(10);
			continue;
		}

		//trail log
		logMaxSize = ReadLogMaxSize(logIniPath);
		if (logMaxSize > 0)
		{
			_maxLogSize = logMaxSize * 1024 * 1024;
			LOG_INFO << "change g_maxLogSize:" << _maxLogSize;
		}
		if (0 != _maxLogSize)
		{
			RecycleLog(_logPath);
		}

		//set log severity
		logSeverity = ReadLogSeverity(logIniPath);
		if (logSeverity < google::GLOG_DEBUG || logSeverity > google::GLOG_FATAL)
		{
			logSeverity = google::GLOG_INFO;
		}

		if (FLAGS_minloglevel != logSeverity)
		{
			FLAGS_minloglevel = logSeverity;
			LOG_INFO << "change FLAGS_minloglevel:" << FLAGS_minloglevel;
		}

		//is changed day
		std::string today = GetLogdayNow();
		if (today != FLAGS_logday_now)
		{
			FLAGS_logday_now = today;
		}
	}
}

#ifdef _WIN32
void LogUtils::RecycleLog(const std::string &logPath)
{
	std::list<fileInfo> fileList;
	unsigned long long totalSize = 0;

	//find
	struct _finddata_t files;
	int fileHandle = 0;

	std::string findPath = logPath + "/*.*";
	fileHandle = _findfirst(findPath.c_str(), &files);

	if (fileHandle == -1)
	{
		fprintf(stderr, "_findfirst error\n");
		return;
	}

	do
	{
		//xxxx_info.log xxx_info.log.20170222
		const char *logStr = strstr(files.name, ".log");
		if ((logStr != NULL) && (strlen(logStr) > 8))//found and not like xx.log
		{
			fileInfo file;
			strcpy(file.path, logPath.c_str());
			strcat(file.path, "/");
			strcat(file.path, files.name);
			file.size = files.size;
			file.modifyTime = files.time_write;

			fileList.push_back(file);
			totalSize += file.size;
		}
	} while (0 == _findnext(fileHandle, &files));

	_findclose(fileHandle);
	fileHandle = NULL;

	//按时间从小到大排序
	fileList.sort();

	//如果超过长度，则优先删除旧的文件
	for (std::list<fileInfo>::iterator iter = fileList.begin(); (totalSize > _maxLogSize) && iter != fileList.end(); ++iter)
	{
		if (0 == remove((*iter).path))
		{
			totalSize -= (*iter).size;
			LOG_INFO << "totalSize:" << totalSize << " | delete file:" << (*iter).path;
		}
		else
		{
			LOG_INFO << "delete file faild:" << (*iter).path;
		}
	}
}
#else
void LogUtils::RecycleLog(const std::string &logPath)
{
	std::list<fileInfo> fileList;
	unsigned long long totalSize = 0;

	DIR *dp = NULL;
	struct dirent *entry = NULL;
	struct stat statbuf;

	if ((dp = opendir(logPath.c_str())) == NULL)
	{
		fprintf(stderr, "opendir error\n");
		return;
	}

	while ((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &statbuf);

		if (!S_ISDIR(statbuf.st_mode))//not a dir
		{
			//xxxx_info.log xxx_info.log.20170222
			const char *logStr = strstr(entry->d_name, ".log");
			int leftLen = (NULL == logStr) ? 0 : strlen(logStr);

			if (NULL == logStr || leftLen < 8)//not found or like xx.log
			{
				continue;
			}

			if (!S_ISREG(statbuf.st_mode))//is a normal file
			{
				continue;
			}

			fileInfo file;
			strcpy(file.path, logPath.c_str());
			strcat(file.path, "/");
			strcat(file.path, entry->d_name);
			file.size = statbuf.st_size;
			file.modifyTime = statbuf.st_mtime;

			fileList.push_back(file);
			totalSize += file.size;
		}
	}

	closedir(dp);
	dp = NULL;

	//按时间从小到大排序
	fileList.sort();

	//如果超过长度，则优先删除旧的文件
	for (std::list<fileInfo>::iterator iter = fileList.begin(); (totalSize > _maxLogSize) && iter != fileList.end(); ++iter)
	{
		if (0 == remove((*iter).path))
		{
			totalSize -= (*iter).size;
			LOG_INFO << "totalSize:" << totalSize << " | delete file:" << (*iter).path;
		}
		else
		{
			LOG_INFO << "delete file faild:" << (*iter).path;
		}
	}
}
#endif

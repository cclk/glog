#include "logger.h"
#include "utils.h"

static LogUtils *logUtils = nullptr;

int InitLogger(
	const std::string &logName /*= ""*/,
	const std::string &logPath /*= ""*/,
	const unsigned int &maxLogSize /*= 1024*/,
	const unsigned int &perLogSize /*= 10*/)
{
	if (logUtils)
	{
		fprintf(stderr, "glog is already inited.\n");
		return -1;
	}

	//创建日志路径
	std::string localLogPath = LogUtils::CreateLogPath(logPath);
	if (localLogPath.empty())
	{
		fprintf(stderr, "logPath is empty.\n");
		return -2;
	}
	else
	{
		uint64_t localMaxLogSize = maxLogSize * 1024 * 1024;
		logUtils = new LogUtils(localMaxLogSize, localLogPath);
	}

	//设置日志配置
#ifdef _DEBUG
	FLAGS_stderrthreshold = google::GLOG_INFO;//大于或等于这个日志级别的日志除了输出到日志文件外还输出到stderr. INFO: 0 WARNING: 1 ERROR: 2 FATAL: 3
	FLAGS_alsologtostderr = true;//除了输出日志信息到日志文件外,还同时输出到stderr.
	FLAGS_colorlogtostderr = true;//输出彩色的日志信息(需要终端支持).
#endif
	FLAGS_logbufsecs = 0;//最多缓存这么多秒的日志信息
	FLAGS_stop_logging_if_full_disk = true;//如果磁盘已满,停止往磁盘写日志
	FLAGS_max_log_size = perLogSize;//日志文件最大大小(MB). 0值会默认改为1.

	//设置日志级别
	std::string logInitPath = LogUtils::GetAppPath() + "/glog.ini";
	int logSeverity = LogUtils::ReadLogSeverity(logInitPath);
	if (logSeverity < google::GLOG_DEBUG || logSeverity > google::GLOG_FATAL) logSeverity = google::GLOG_INFO;
	FLAGS_minloglevel = logSeverity;

	//设置日志路径
	std::string localLogName = logName;
	if (localLogName.empty()) localLogName = LogUtils::GetAppName();
	std::string debugStr = localLogPath + "/" + localLogName;

	//设置日志后缀
	google::SetLogFilenameExtension(".log");
	//设置存储路径及文件前缀
	google::SetLogDestination(google::GLOG_DEBUG, debugStr.c_str());

	//设置日志切换
	FLAGS_logday_prev = LogUtils::GetLogdayPrev(debugStr + ".log");
	FLAGS_logday_now = LogUtils::GetLogdayNow();
	FLAGS_logday_prev = FLAGS_logday_prev.empty() ? FLAGS_logday_now : FLAGS_logday_prev;

	//初始化日志
	google::InitGoogleLogging(localLogName.c_str());

	return 0;
}

void ReleaseLogger()
{
	if (logUtils)
	{
		delete logUtils;
		logUtils = nullptr;
	}

	google::ShutdownGoogleLogging();
}

void MlogHappened(std::string file, int line, std::string func, int severity, std::string content)
{
	switch (severity)
	{
	case google::GLOG_DEBUG:
		LOG_DEBUG_M << content;
		break;

	case google::GLOG_INFO:
		LOG_INFO_M << content;
		break;

	case google::GLOG_WARNING:
		LOG_WARN_M << content;
		break;

	case google::GLOG_ERROR:
		LOG_ERROR_M << content;
		break;

	case google::GLOG_FATAL:
		LOG_FATAL_M << content;
		break;

	default:
		LOG_INFO_M << content;
		break;
	}
}

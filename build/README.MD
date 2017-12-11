移植glog修改记录
===



###  1、CMakeLists.txt

+ "set (GLOG_SRCS"后增加（line394-397）：

  ```cmake
    build/logger/logger.h
    build/logger/logger.cc
    build/logger/utils.h
    build/logger/utils.cc
  ```

+ "target_link_libraries (utilities_unittest PRIVATE glog)"后增加测试（line571-576）

  ```cmake
    add_executable (glog_test
      build/test/glog_test.cc
    )
    
    target_link_libraries (glog_test PRIVATE glog)
  ```



### 2、src/glog/log_severity.h

+ "const int GLOG_INFO"后增加级别（line47-48）

  ```c++
  const int GLOG_DEBUG = 0, GLOG_INFO = 1, GLOG_WARNING = 2, GLOG_ERROR = 3, GLOG_FATAL = 4,
    NUM_SEVERITIES = 5;
  ```

  ​

### 3、src/glog/logging.h.in

+ "DECLARE_string(log_dir)"后增加日志切换检测（line364-366）

  ```c++
  DECLARE_string(logday_prev);
  DECLARE_string(logday_now);
  ```

+ "LogMessage(const char* file, int line, LogSeverity severity)"后增加日志打印函数1（line1210）

  ```c++
    LogMessage(const char* file, int line, const char* func, LogSeverity severity);
  ```

+ "void Init(const char* file"后增加日志打印函数2（line1272）

  ```c++
    void Init(const char* file, int line, LogSeverity severity,
              void (LogMessage::*send_method)(), const char* func = nullptr);
  ```



### 4、src/logging.cc

+ "GLOG_DEFINE_string(log_dir"后日志切换检测（line168 - 169）

  ```c++
  GLOG_DEFINE_string(logday_prev, "", "logday_prev");
  GLOG_DEFINE_string(logday_now, "", "logday_now");
  ```

+ "int64 LogMessage::num_messages_"后日志级别（line371 - 379）

  ```c++
  // Number of messages sent at each severity.  Under log_mutex.
  int64 LogMessage::num_messages_[NUM_SEVERITIES] = {0, 0, 0, 0, 0};

  // Globally disable log writing (if disk is full)
  static bool stop_writing = false;

  const char*const LogSeverityNames[NUM_SEVERITIES] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
  };
  ```

+ "bool CreateLogfile(const string& time_pid_string)"后日志切换（line447 - 450）

  ```c++
    
    // Roll day file
    void RollLogFile();
  ```

+ "bool LogFileObject::CreateLogfile(const string"后日志文件名、日志追加（line907 - 921）

  ```c++
    string string_filename = base_filename_+filename_extension_;
                             
    const char* filename = string_filename.c_str();
    int fd = open(filename, O_RDWR | O_EXCL | O_APPEND, FLAGS_logfile_mode);//追加
    if (fd == -1) fd = open(filename, O_RDWR | O_EXCL | O_CREAT, FLAGS_logfile_mode);//创建
    if (fd == -1) return false;
  #ifdef HAVE_FCNTL
    // Mark the file close-on-exec. We don't really care if this fails
    fcntl(fd, F_SETFD, FD_CLOEXEC);
  #endif

    //file size
    file_length_ = lseek(fd, 0, SEEK_END);
  ```

+ "void LogFileObject::Write("后日志切换1（line981 - 989）

  ```c++
    if (static_cast<int>(file_length_ >> 20) >= MaxLogSize() ||
        PidHasChanged() || FLAGS_logday_now != FLAGS_logday_prev) {
      if (file_ != NULL) fclose(file_);
      file_ = NULL;
      file_length_ = bytes_since_flush_ = dropped_mem_length_ = 0;
      rollover_attempt_ = kRolloverAttemptFrequency-1;
      RollLogFile();
    }
  ```

  ​

+ "void LogFileObject::Write("后日志切换2（line1143 - 1160）

  ```c++
  void LogFileObject::RollLogFile() {
    if(FLAGS_logday_prev.empty() || FLAGS_logday_now.empty() || base_filename_.empty()) return;
    
    std::string oldfile = base_filename_ + filename_extension_;
    static char newfile[1024] = { 0 };    
    int index = 0;
    
    while(true) {
      sprintf(newfile, "%s-%s-%d%s", base_filename_.c_str(), FLAGS_logday_prev.c_str(), ++index, filename_extension_.c_str()); 
      if (0 != access(newfile, 0)) {
        rename(oldfile.c_str(), newfile);
        break;
      }
    }
    
    FLAGS_logday_prev = FLAGS_logday_now;
  }
  ```

+ "LogMessage::LogMessage(const char* file, int line"后日志打印函数1（line1212 - 1216）

  ```c++
  LogMessage::LogMessage(const char* file, int line, const char* func, LogSeverity severity)
      : allocated_(NULL) {
    Init(file, line, severity, &LogMessage::SendToLog, func);
  }
  ```

  ​

+ "void LogMessage::Init(const char* file,"后日志打印函数2（line1239 - 1243 ）

  ```c++
  void LogMessage::Init(const char* file,
                        int line,
                        LogSeverity severity,
                        void (LogMessage::*send_method)(),
                        const char* func /*= nullptr*/) {
  ```

  ​

+ "void LogMessage::Init(const char* file,"后日志打印格式（line1296 - 1338）

  ```c++
   if (FLAGS_log_prefix && (line != kNoLogPrefix)) {
      if(func) { 
          stream() << '['
              << LogSeverityNames[severity]
              << ' '
              << setw(4) << 1900 + data_->tm_time_.tm_year
              << '-'
              << setw(2) << 1 + data_->tm_time_.tm_mon
              << '-'
              << setw(2) << data_->tm_time_.tm_mday
              << ' '
              << setw(2) << data_->tm_time_.tm_hour << ':'
              << setw(2) << data_->tm_time_.tm_min << ':'
              << setw(2) << data_->tm_time_.tm_sec << "."
              << setw(3) << usecs
              << ' '
              << setfill(' ') << setw(5)
              << static_cast<unsigned int>(GetTID()) << setfill('0')
              << ' '
              << data_->basename_ 
              << ' '
              << '(' << data_->line_ << ')'
              << ' ' << func << "] ";
      } else {
          stream() << '[' 
              << LogSeverityNames[severity]
              << ' '
              << setw(4) << 1900 + data_->tm_time_.tm_year
              << '-'
              << setw(2) << 1 + data_->tm_time_.tm_mon
              << '-'
              << setw(2) << data_->tm_time_.tm_mday
              << ' '
              << setw(2) << data_->tm_time_.tm_hour << ':'
              << setw(2) << data_->tm_time_.tm_min << ':'
              << setw(2) << data_->tm_time_.tm_sec << "."
              << setw(3) << usecs
              << ' '
              << setfill(' ') << setw(5)
              << static_cast<unsigned int>(GetTID()) << setfill('0')
              << ' '
              << data_->basename_ << ':' << data_->line_ << "] ";
      }         
    }
  ```

  ​




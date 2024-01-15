#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include "singleton.h"
#include "thread.h"
#include "util.h"
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <vector>

#define SYLAR_LOG_LEVEL(logger, level)                                         \
    if (logger->getLevel() <= level)                                           \
    sylar::LogEventWrap(                                                       \
        sylar::LogEvent::ptr(new sylar::LogEvent(                              \
            logger, level, __FILE__, __LINE__, 0, sylar::GetThreadId(),        \
            sylar::GetFiberId(), time(0))))                                    \
        .getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)                           \
    if (logger->getLevel() <= level)                                           \
    sylar::LogEventWrap(                                                       \
        sylar::LogEvent::ptr(new sylar::LogEvent(                              \
            logger, level, __FILE__, __LINE__, 0, sylar::GetThreadId(),        \
            sylar::GetFiberId(), time(0))))                                    \
        .getEvent()                                                            \
        ->format(fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...)                                  \
    SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...)                                   \
    SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...)                                   \
    SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...)                                  \
    SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...)                                  \
    SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar {

class Logger;
class LoggerManager;

// 日志级别
class LogLevel {
  public:
    enum Level { // 枚举
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    // 静态成员函数，
    // 与实例无关。不需要通过对象来动用，可以直接通过类名或类的对象来调用。
    // 用来实现与类相关的操作而不是特定的对象实例相关的操作
    // 静态成员函数可以访问类的静态数据成员和其他静态成员函数，但不能直接访问普通成员变量和成员函数，
    // 因为静态成员函数没有this指针。
    // 此外，静态成员函数不能声明为const的，也不能使用virtual和override关键字。
    // 静态成员函数的声明和定义中通常不需要指定static关键字。
    // 用来将等级转换成字符串
    // 调用静态成员函数可以使用类名或类的对象
    static const char *ToString(LogLevel::Level level);
    // 从字符串得到等级
    static LogLevel::Level FromString(const std::string &str);
};

// 日记事件
class LogEvent {
  public:
    // 别名 智能指针
    // std::shared_ptr通过一个引用计数来跟踪有多少个std::shared_ptr指向相同的对象。
    // 每当创建shared_ptr指向一个对象时，引用计数会增加；当std::shared_ptr超出作用域、被重置或赋予一个新值时
    // 或者通过std::shared_ptr的reset方法手动释放对象，引用计数会减少。
    // 只有当引用计数为零时，std::shared_ptr才会释放所指向的对象。
    // std::shared_ptr还可以自动确保在多线程环境下对所指向对象的安全访问，
    // 使用引用计数来跟踪引用关系可能会导致引用循环（循环引用），从而导致对象永远无法被释放。
    // 为避免出现这种情况，可以使用std::weak_ptr来打破循环引用
    using ptr = std::shared_ptr<LogEvent>;
    // 构造函数
    // 构造函数在创建对象时被自动调用，用于对对象的成员变量进行初始化。
    // 构造函数的名称与类名相同，不返回任何类型，也不需要显式声明返回类型。
    // 构造函数可以有多个重载版本，它们的参数个数或类型不同。
    // 如果没有显式地定义构造函数，编译器会为类生成默认构造函数，该默认构造函数不带参数，并执行空操作
    // 在派生类中，可以通过初始化列表调用基类的构造函数，以确保基类部分的对象在派生类构造函数执行之前先被构造；
    // 同时，派生类的析构函数会在基类的析构函数执行完毕后被调用
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
             const char *file, int32_t m_line, uint32_t elapse,
             uint32_t thread_id, uint32_t fiber_id, uint64_t time);
    // 一系列访问私有成员变量的操作
    const char *getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_ss.str(); }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }
    std::stringstream &getSS() { return m_ss; }

    void format(const char *fmt, ...);
    void format(const char *fmt, va_list al);

  private:
    const char *m_file = nullptr; // 文件名
    int32_t m_line = 0;           // 行号
    uint32_t m_elapse = 0;        // 程序启动到现在的ms
    uint32_t m_threadId = 0;      // 线程id
    uint32_t m_fiberId = 0;       // 协程id
    u_int64_t m_time = 0;         // 时间戳
    std::stringstream m_ss;       // 内容

    std::shared_ptr<Logger> m_logger; // 日志器
    LogLevel::Level m_level;          // 日志级别
};

// 日志事件包装器
class LogEventWrap {
  public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    std::stringstream &getSS();
    LogEvent::ptr getEvent() const { return m_event; }

  private:
    LogEvent::ptr m_event;
};

// 日志格式器
class LogFormatter {
  public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string &pattern);

    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                       LogEvent::ptr event);

  public:
    class FormatItem { // 格式类型
      public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem() {}
        virtual void format(std::ostream &os, std::shared_ptr<Logger> logger,
                            LogLevel::Level level, LogEvent::ptr event) = 0;
    };
    void init();
    bool isError() const { return m_error; }
    const std::string getPattern() const { return m_pattern; }

  private:
    std::string m_pattern;                // 日志格式
    std::vector<FormatItem::ptr> m_items; // 存放各种类型的格式的数组
    bool m_error = false;
};

// 日志输出地
class LogAppender {
    friend class Logger;

  public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter() const { return m_formatter; }

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

  protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
    bool m_hasFormatter = false;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
    friend class LoggerManager;

  public:
    using ptr = std::shared_ptr<Logger>;

    Logger(const std::string &name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

    const std::string &getName() const { return m_name; }

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string &val);
    LogFormatter::ptr getFormatter();

    std::string toYamlString();

  private:
    std::string m_name;                      // 日志名称
    LogLevel::Level m_level;                 // 日志级别
    std::list<LogAppender::ptr> m_appenders; // appender集合
    LogFormatter::ptr m_formatter;
    Logger::ptr m_root;
};

// 输出到控制台的
class StdoutLogAppender : public LogAppender {
  public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    void log(Logger::ptr logger, LogLevel::Level level,
             LogEvent::ptr event) override;
    std::string toYamlString() override;
};

// 输出到文件的
class FileLogAppender : public LogAppender {
  public:
    using ptr = std::shared_ptr<FileLogAppender>;
    FileLogAppender(const std::string &filename);
    void log(Logger::ptr logger, LogLevel::Level level,
             LogEvent::ptr event) override;

    std::string toYamlString() override;
    bool reopen();

  private:
    std::string m_filename;
    std::ofstream m_filestream;
};

class LoggerManager {
  public:
    LoggerManager();
    Logger::ptr getLogger(const std::string &name);

    void init();
    Logger::ptr getRoot() const { return m_root; }
    std::string toYamlString();

  private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

using LoggerMgr = sylar::singleton<LoggerManager>;

} // namespace sylar

#endif

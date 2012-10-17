/**
 * @file   Log.h
 * @author wudongzheng01 <donzell.wu@gmail.com>
 * @date   Sat Sep 22 20:42:59 2012
 * 
 * @brief  С�ͼ򵥵���־�⣬�ܹ�����С�з���־��֧�ֶ��̡߳������дͬһ����־�ļ��������ܵĸ�Ч����Ҫ��Log��Ϊ����ƿ����
 * ���Զ�̬����ĳһ��logʵ����level���������������þͲ��ܶ�̬�ı��ˣ�����ӵ�ж��logʵ������־��ʽ������;֧�ֳ�����level;
 * 
 * 
 */
#ifndef _LOG_H_
#define _LOG_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <string>
#include <map>
#include "Formatter.h"
#include "Appender.h"
#include "FileAppender.h"
#include "AsyncFileAppender.h"
#include "LogStream.h"

using std::map;
using std::string;
using std::make_pair;
using std::pair;


class CLogger;
typedef CLogger* LoggerPtr;

enum
{
    FATAL=0,
    WARN,
    ERROR,
    NOTICE,
    TRACE,
    LOG,
    INFO,
    DEBUG,
    LEVEL_ALL,
};

extern const char* LEVEL_STR[LEVEL_ALL];


class CLogger:boost::noncopyable
{
  public:
    enum{
        MAX_SIZE_PER_LOG=4096,
    };
    
    /** 
     * ֻӦ�ó�ʼ��һ��
     * 
     * @param path �����ļ�·��
     * 
     * @return �ɹ�0������ʧ��
     */
    static int32_t init(const string& path);
    /** 
     * �̰߳�ȫ�Ļ�ȡһ��Logʵ��
     * 
     * @param logname 
     * 
     * @return 
     */
    static LoggerPtr getLogInstance(const string& logname);


    void setLogLevel(int level){level_ = level;}
    int getLogLevel()const{return level_;}
    inline bool canLog(int level)
        {return level <= level_;}
    
    void setLogName(const string& logname)
        {logname_ = logname;}
    
    const string getLogName() const {return logname_;}
    
    Appender* getAppender()const
    {
        return pAppender_;
    }
    
    Formatter& getFormatter(){return formatter_;}
    
    
    void writeLog(const char* file,int line,const char* func,int level,const char* fmt,...)__attribute__((format(printf,6,7)));
    void output(const char* msg,size_t size);

    class LogTemp
    {
      public:
      LogTemp(CLogger& logger,Formatter& formatter,const char* file,int line,const char* func,int level)
          :logger_(logger),formatter_(formatter),file_(file),line_(line),func_(func),level_(level)
        {
            formatter_.formatBeforeMsg(stream_,logger_.getLogName().c_str(),file_,line_,func_,level_);
        }
        
        ~LogTemp(){
            formatter_.formatAfterMsg(stream_,logger_.getLogName().c_str(),file_,line_,func_,level_);
            stream_.forceAddEndLine();
            logger_.output(stream_.buffer().data(),stream_.buffer().length());
        }

        LogStream& getLogStream(){
            return stream_;
        }
        
      private:
        CLogger& logger_;
        Formatter& formatter_;
        const char* file_;
        int line_;
        const char* func_;
        int level_;
        LogStream stream_;
    };

  private:
    CLogger(const string& logname,Appender* pAppender,const string& fmt,int level);
    ~CLogger();
    static void destroy();
    

    // 1.���ָ���и��ֶ�
    // 2.trim�����ֶ�
    // 3.��һ�����������ֶβ���Ϊ�գ��������зִ�С��Ĭ��2G��������level��Ĭ��ȫ����    
    static std::vector<std::string> splitCheck(const std::string& instance);
    
    
    string logname_;
    // CLogAppender* appender_;
    Formatter formatter_;
    volatile int level_;
    Appender* pAppender_;
    
    static map<string,CLogger*> logMap_;
    typedef map<string,CLogger*>::iterator  logMapIter;
    static boost::mutex mutex_;
    static map<string,Appender*> appenderMap_;

};

#define LOG(logger,level) if(logger && logger->canLog(level)) CLogger::LogTemp(*logger,logger->getFormatter(),__FILE__,__LINE__,__func__,level).getLogStream()

#define LOG_FATAL(logger,fmt,...)                                       \
    do{                                                                 \
        if(logger && logger->canLog(FATAL))                    \
            logger->writeLog(__FILE__,__LINE__,__func__,FATAL,fmt,##__VA_ARGS__); \
    }while(0)


#define LOG_WARN(logger,fmt,...)                                        \
    do{                                                                 \
        if(logger && logger->canLog(WARN))                    \
            logger->writeLog(__FILE__,__LINE__,__func__,WARN,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOG_ERROR(logger,fmt,...)                                       \
    do{                                                                 \
        if(logger && logger->canLog(ERROR))                    \
            logger->writeLog(__FILE__,__LINE__,__func__,ERROR,fmt,##__VA_ARGS__); \
    }while(0)


#define LOG_TRACE(logger,fmt,...)                                        \
    do{                                                                 \
        if(logger && logger->canLog(TRACE))                    \
            logger->writeLog(__FILE__,__LINE__,__func__,TRACE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOG_NOTICE(logger,fmt,...)                                        \
    do{                                                                 \
        if(logger && logger->canLog(NOTICE))                   \
            logger->writeLog(__FILE__,__LINE__,__func__,NOTICE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOG_LOG(logger,fmt,...)                                         \
    do{                                                                 \
        if(logger && logger->canLog(LOG))                    \
            logger->writeLog(__FILE__,__LINE__,__func__,LOG,fmt,##__VA_ARGS__); \
    }while(0)

#define LOG_INFO(logger,fmt,...)                                        \
    do{                                                                 \
        if(logger && logger->canLog(INFO))                    \
            logger->writeLog(__FILE__,__LINE__,__func__,INFO,fmt,##__VA_ARGS__); \
    }while(0)


#define LOG_DEBUG(logger,fmt,...)                                        \
    do{                                                                 \
        if(logger && logger->canLog(DEBUG))                    \
            logger->writeLog(__FILE__,__LINE__,__func__,DEBUG,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#endif
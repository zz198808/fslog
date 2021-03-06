#ifndef _FILEAPPENDER_H_
#define _FILEAPPENDER_H_

#include <string>
#include <stdint.h>
#include <vector>
#include <map>
#include <boost/thread.hpp>
#include "Appender.h"

class FileAppender:public Appender
{
public:
    enum
    {
        MIN_SPLITSIZE=20*1024*1024UL, // 最小20M
        MAX_SPLITSIZE=8*1024*1024*1024UL, // 最大8G
        DEFAULT_SPLITSIZE=2*1024*1024*1024UL, // 默认2G
    };
    typedef struct format
    {
        enum{
            RAW_STR,
            YEAR,
            MONTH,
            DAY,
            HOUR,
            MINUTE,
            SECOND,
            PID,
            SEQNUMBER,
        };
        int type_;
        std::string str_;
    }format_t;
    
    
    FileAppender(const std::string& path,uint64_t splitsize,const std::string& splitFormat);
    void setCheckTimeInterval(time_t seconds){checkTimeInterval_=seconds;}
    
    void setSplitSize(uint64_t splitsize){
        if(splitsize <= 0)
            splitSize_ = DEFAULT_SPLITSIZE;
        else if(splitsize < MIN_SPLITSIZE)
            splitSize_ = MIN_SPLITSIZE;
        else if(splitsize > MAX_SPLITSIZE)
            splitSize_ = MAX_SPLITSIZE;
        else
            splitSize_ = splitsize;
    }
    uint64_t getSplitSize()const{return splitSize_;}

    void setSplitFormat(const std::string& splitFormat)
    {
        splitFormat_ = splitFormat;
        parseSplitFormat();     // 要更新splitFormat
    }
    std::string getSplitFormat()const{return splitFormat_;}
    
    void reopen();
    /* 负责检查文件是否该切分了，是否已经被mv走了，必要时移走旧文件，生成新文件 */
    void checkFile();
    int getFd()const
        {return fd_;}
    
    virtual ~FileAppender();
    
    virtual bool start()
        {
            // 如果没能成功打开日志文件，这里返回失败。我们要保证一直持有一个有效fd。
            return (fd_ >= 0);
        }
    
    virtual void stop()
        {}
    
    virtual void output(char* msg,size_t len);
    void outputWithoutLock(char* msg,size_t len);

    std::string getNewFilename(time_t now,int seqNumber)const;
private:
    void parseSplitFormat();
    void generateSplitFormatCharMap();
    int openLogFile(const std::string& filePath);
    
    std::string path_;
    uint64_t splitSize_;
    boost::mutex checkMutex_;
    time_t checkTimeInterval_;  // 每几秒检查一次文件
    time_t lastcheck_;          // 上次检查时间
    int fd_;
    std::string splitFormat_;
    std::vector<format_t> formats_;
    typedef std::map<char,int> charToFormatMap;
    charToFormatMap formatsCharMap_;
    boost::mutex splitMutex_;
    
};

#endif /* _FILEAPPENDER_H_ */

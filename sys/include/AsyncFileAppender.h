#ifndef _ASYNCFILEAPPENDER_H_
#define _ASYNCFILEAPPENDER_H_

#include <string>
#include <vector>
#include <pthread.h>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "FileAppender.h"

class AsyncFileAppender;
typedef void (*cookieFunc)();
void magicCookieFunc();

typedef struct asyncWriteItem
{
    AsyncFileAppender* pAppender_;
    std::string buffer_;
    asyncWriteItem(AsyncFileAppender* pAppender,const std::string& buffer)
        :pAppender_(pAppender),buffer_(buffer),pCookie_(magicCookieFunc)
        {}
    asyncWriteItem(AsyncFileAppender* pAppender,const char* msg,size_t len)
        :pAppender_(pAppender),buffer_(msg,len),pCookie_(magicCookieFunc)
        {}
    ~asyncWriteItem()
        {
            // must clear it.delete will not erase memory.
            pCookie_ = NULL;
        }
    cookieFunc pCookie_;
}asyncWriteItem_t;

// 为了减少线程竞争，一个AsyncFileAppender有一个单独的异步线程，不公用一个异步线程，这样不会出现多个Append和一个线程竞争锁，push数据到队列
class AsyncFileAppender:public FileAppender
{
public:
  AsyncFileAppender(const std::string& path,size_t splitSize,std::string& splitFormat)
      :FileAppender(path,splitSize,splitFormat),asyncRunning_(false)
        {
        }
    virtual ~AsyncFileAppender()
        {}
    virtual bool start();
    virtual void stop();

    virtual void output(const std::string& msg);
    virtual void output(const char* msg,size_t len);

    void inThreadOutput(const char* msg,size_t len);
    
  private:
    boost::scoped_ptr<boost::thread> thread_;
    enum{
        flushInterval_=3,
    };
    
    volatile bool asyncRunning_;
    boost::mutex queueMutex_;
    boost::mutex runMutex_;
    
    std::vector<asyncWriteItem_t* > logQueue_;
    void threadFunc();
    static void s_dumpQueue(void* arg);
    void dumpQueue();
    
    
};


#endif /* _ASYNCFILEAPPENDER_H_ */

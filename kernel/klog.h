#ifndef __KLOG_H_
#define __KLOG_H_

#include <stdarg.h>


enum 
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
};

#define LOG_LEVEL(...)                          \
    do                                          \
    {                                           \
        klog_lock_acquire();                    \
        klog("%s:%d | ", __FILE__, __LINE__);   \
        klog_level(__VA_ARGS__);                \
        klog_putchar('\n');                     \
        klog_lock_release();                    \
    }while(0)


#define LOG_DEBUG(...) LOG_LEVEL(LOG_LEVEL_DEBUG, __VA_ARGS__);
#define LOG_INFO(...) LOG_LEVEL(LOG_LEVEL_INFO, __VA_ARGS__);
#define LOG_WARNING(...) LOG_LEVEL(LOG_LEVEL_WARNING, __VA_ARGS__);
#define LOG_ERROR(...) LOG_LEVEL(LOG_LEVEL_ERROR, __VA_ARGS__);

#define LOG_ASSERT(x)                           \
    do                                          \
    {                                           \
        if(!(x)) {                                \
            LOG_ERROR("assert \"" #x "\" failed.");\
        }                                       \
    }while (0)

#define LOG_VAR(x)                              \
    do                                          \
    {                                           \
        LOG_INFO("%s = %x", #x, x);             \
    }while(0)
    

void klog_init();
void klog_putchar(char ch);
int klog(const char *format, ...);
int klog_va(const char *format, va_list ap);
void klog_level(int level, const char *format, ...);

void klog_lock_init();
void klog_lock_acquire();
void klog_lock_release();

#endif 

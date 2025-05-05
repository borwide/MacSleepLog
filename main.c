#include <stdio.h>
#include <time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>
#include <stdlib.h>

#define LOG_FILE "macsleepmon.log"

// 全局变量
io_connect_t root_port;
FILE *log_file;

// 系统电源状态变化的回调函数
void systemPowerCallback(void *refCon, io_service_t service, natural_t messageType, void *messageArgument) {
    time_t now;
    char timestr[64];
    const char *message = NULL;
    
    switch (messageType) {
        case kIOMessageSystemWillSleep:
            message = "系统即将进入休眠状态";
            now = time(NULL);
            strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));
            printf("[%s] %s\n", timestr, message);
            if (log_file) {
                fprintf(log_file, "[%s] %s\n", timestr, message);
                fflush(log_file);
            }
            IOAllowPowerChange(root_port, (long)messageArgument);
            break;
            
        case kIOMessageCanSystemSleep:
            IOAllowPowerChange(root_port, (long)messageArgument);
            break;
            
        case kIOMessageSystemHasPoweredOn:
            message = "系统已从休眠状态唤醒";
            now = time(NULL);
            strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));
            printf("[%s] %s\n", timestr, message);
            if (log_file) {
                fprintf(log_file, "[%s] %s\n", timestr, message);
                fflush(log_file);
            }
            break;
            
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    io_connect_t root_port;
    IONotificationPortRef notifyPortRef;
    io_object_t notifierObject;
    time_t now;
    char timestr[64];
    const char *message;
    
    // 打开日志文件
    log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        printf("错误：无法打开日志文件 %s\n", LOG_FILE);
        return 1;
    }
    
    // 打开根端口
    root_port = IORegisterForSystemPower(NULL, &notifyPortRef, systemPowerCallback, &notifierObject);
    if (root_port == 0) {
        now = time(NULL);
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));
        message = "错误：无法注册系统电源状态监控";
        printf("[%s] %s\n", timestr, message);
        fprintf(log_file, "[%s] %s\n", timestr, message);
        fclose(log_file);
        return 1;
    }
    
    // 添加端口到运行循环
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                      IONotificationPortGetRunLoopSource(notifyPortRef),
                      kCFRunLoopDefaultMode);
    
    {
        time_t now = time(NULL);
        char timestr[64];
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));
        printf("[%s] 开始监控系统休眠和唤醒事件...\n按Ctrl+C退出\n", timestr);
    }
    
    // 运行主循环
    CFRunLoopRun();
    
    // 清理资源
    IODeregisterForSystemPower(&notifierObject);
    IOServiceClose(root_port);
    IONotificationPortDestroy(notifyPortRef);
    
    if (log_file) {
        fclose(log_file);
    }
    
    return 0;
}
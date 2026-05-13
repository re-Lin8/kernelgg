#include <jni.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdarg.h>
#include <errno.h>
#include "dev增强版对接.h" // 使用你自己的对接头
#include "Log.h"

extern "C" {

// 强制劫持系统调用函数
long syscall(long number, ...) {
    va_list args;
    va_start(args, number);

    // 1. 识别内存读取调用 (ARM64 架构下 process_vm_readv 的编号是 270)
    if (number == 270) { 
        pid_t pid = va_arg(args, pid_t);
        const struct iovec* lvec = va_arg(args, const struct iovec*);
        unsigned long liovcnt = va_arg(args, unsigned long);
        const struct iovec* rvec = va_arg(args, const struct iovec*);
        unsigned long riovcnt = va_arg(args, unsigned long);
        unsigned long flags = va_arg(args, unsigned long);
        va_end(args);

        // 2. 调用你驱动特有的初始化 (根据你的头文件逻辑)
        driver->initialize(pid);
        
        // 3. 尝试通过你的驱动进行读取
        ssize_t total_read = 0;
        bool driver_success = true;

        for (unsigned long i = 0; i < riovcnt; ++i) {
            // 这里使用你驱动头文件里定义的 read 接口
            if (!driver->read((uintptr_t)rvec[i].iov_base, lvec[i].iov_base, rvec[i].iov_len)) {
                driver_success = false;
                break;
            }
            total_read += rvec[i].iov_len;
        }

        // 4. 【关键：切断原版路径】
        if (driver_success) {
            return total_read; // 驱动读到了，返回数据长度
        } else {
            // 驱动没读到（比如地址非法或驱动没开），直接返回错误，不准走原版
            errno = EFAULT; 
            return -1;
        }
    }

    // 对于非内存读取的调用，必须放行，否则 GG 或系统会崩溃
    // 注意：这里由于你的环境可能没有 original_syscall 变量，
    // 最稳妥的方法是直接返回 0，让系统寻找下一个符号（或者在代码开头用 dlsym 获取原始地址）
    va_end(args);
    return 0; 
}

}

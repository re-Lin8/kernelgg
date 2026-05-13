#include <jni.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/types.h>
#include "dev增强版对接.h" // 确保你的头文件在同目录下

// 保持与原项目一致的宏定义
#define LOG_TAG "FIX_GG"
#include "Log.h" 

extern "C" {

/**
 * 核心修改：将 GG 的内存读取请求重定向到你的驱动
 */
ssize_t vm_readv(pid_t pid, const struct iovec* lvec, unsigned long liovcnt, 
                 const struct iovec* rvec, unsigned long riovcnt, unsigned long flags) {
    
    // 1. 初始化驱动对应的 PID
    // 每次读取时校准 PID，确保驱动操作的目标正确
    driver->initialize(pid);

    ssize_t total_read = 0;

    // 2. 遍历 GG 请求的所有内存段 (rvec 是远程地址，lvec 是本地缓冲区)
    for (unsigned long i = 0; i < riovcnt; ++i) {
        uintptr_t remote_addr = (uintptr_t)rvec[i].iov_base;
        void* local_buf = lvec[i].iov_base;
        size_t size = rvec[i].iov_len;

        // 3. 调用你的驱动进行读取
        if (driver->read(remote_addr, local_buf, size)) {
            total_read += size;
        } else {
            LOGE("驱动读取失败: PID=%d, Addr=%p, Size=%zu", pid, (void*)remote_addr, size);
            // 如果读取失败，返回 -1 告知 GG
            return -1; 
        }
    }

    return total_read;
}

/**
 * 如果你需要同时也修复写入功能，可以实现此函数
 */
ssize_t vm_writev(pid_t pid, const struct iovec* lvec, unsigned long liovcnt, 
                  const struct iovec* rvec, unsigned long riovcnt, unsigned long flags) {
    
    driver->initialize(pid);
    ssize_t total_written = 0;

    for (unsigned long i = 0; i < riovcnt; ++i) {
        uintptr_t remote_addr = (uintptr_t)rvec[i].iov_base;
        void* local_buf = lvec[i].iov_base;
        size_t size = rvec[i].iov_len;

        if (driver->write(remote_addr, local_buf, size)) {
            total_written += size;
        } else {
            return -1;
        }
    }
    return total_written;
}

}

#pragma once

#include <cstdint>
#include <cstddef>

// ============================================================================
// MuFile — 文件 I/O 抽象层（替代 Win32 CreateFile/ReadFile + fopen/fread）
// ============================================================================
//
// Android 文件访问路由:
//   - "Data/..." → AAssetManager (只读 assets)
//   - 绝对路径 (以 / 开头) → fopen/fread (内部存储/外部存储)
//   - 相对写入路径 → 基于 m_internalPath 的 fopen/fwrite
//
// 使用方式:
//   void* f = MuFile::open("Data/Local/Dec2.dat", "rb");
//   long sz = MuFile::size(f);
//   MuFile::read(f, buffer, sz);
//   MuFile::close(f);
//
//   // 或一次性读取:
//   size_t size;
//   uint8_t* data = MuFile::readAll("Data/Local/Dec2.dat", size);

namespace MuFile {

// 初始化 (assetManager 可为 nullptr 仅当不需要 asset 访问)
void init(void* assetManager, const char* internalPath);

// 打开文件
// path: 相对路径 (Data/...) → assets, 绝对路径 → 磁盘
// mode: "rb" (只读), "wb" (写入), "ab" (追加)
// 返回: 文件句柄 (nullptr = 失败)
void* open(const char* path, const char* mode = "rb");

// 读取数据
// 返回: 实际读取的字节数
size_t read(void* handle, void* buffer, size_t size);

// 获取文件大小
long size(void* handle);

// 定位
// origin: SEEK_SET (0), SEEK_CUR (1), SEEK_END (2)
int seek(void* handle, long offset, int origin);

// 当前偏移
long tell(void* handle);

// 关闭文件
void close(void* handle);

// 检查文件是否存在
bool exists(const char* path);

// 一次性读取整个文件到堆内存
// outSize: 输出文件大小
// 返回: 堆分配的缓冲区 (调用者用 freeBuffer 释放)
uint8_t* readAll(const char* path, size_t& outSize);

// 释放 readAll 返回的缓冲区
void freeBuffer(uint8_t* buffer);

// 写入数据到磁盘文件
// 返回: 实际写入的字节数
size_t write(void* handle, const void* buffer, size_t size);

// 确保目录存在 (磁盘路径)
bool makeDir(const char* path);

// 获取内部存储路径
const char* getInternalPath();

} // namespace MuFile

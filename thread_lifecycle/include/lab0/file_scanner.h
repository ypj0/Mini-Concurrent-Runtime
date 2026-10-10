#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <thread>
#include <utility>
#include <vector>

#include "worker_stats.h"

namespace lab0 {

/// 并行文件扫描器：把 root 下所有文件分片给 N 个 worker 线程扫描，汇总统计。
///
/// 这是 Lab 0 的主载体，Milestone 1/3/4 都围绕 scan() 演进（接口不变，内部实现替换）：
///   - MS1：裸 std::thread + 全局 atomic 统计，跑通分片
///   - MS2：改用 JoiningThread（替换 scan() 里的裸 thread）
///   - MS3：审查参数捕获，消除悬空引用（值捕获 / move）
///   - MS4：全局 atomic 换成每 worker 的局部 WorkerStats，主线程汇总
class FileScanner {
  public:
    FileScanner(std::filesystem::path root, std::size_t num_workers)
        : root_path_(std::move(root)), num_workers_(num_workers), fileCount_(0), allfileBytes_(0)
    {
      num_workers_ = num_workers_ == 0 ? 1 : num_workers_;
    }

    /// 扫描 root 目录，返回所有文件的汇总统计。
    ///
    // TODO(MS1): 用 recursive_directory_iterator 收集所有 regular_file 路径，
    //            按 num_workers_ 分片，每 worker 一段，裸 std::thread 并行扫描，
    //            全局 std::atomic 累计 files_scanned / total_bytes。
    // TODO(MS2): 把 std::thread 换成 lab0::JoiningThread，删除手工 join 循环。
    // TODO(MS3): 审查 worker 的参数捕获——确保每 worker 拿到独立的文件列表副本
    //            （值捕获或 std::move），不捕获可能悬空的引用；worker_id 用值捕获。
    // TODO(MS4): 去掉全局 atomic，每 worker 用局部 WorkerStats 统计，写回
    //            预分配的 std::vector<WorkerStats> 对应槽位，主线程汇总。
    WorkerStats scan() {
      fileCount_ = 0;
      allfileBytes_ = 0;
      threads.clear();

      if(all_files.size() == 0) {
        for(const auto& entry : std::filesystem::recursive_directory_iterator(root_path_)) {
          if (entry.is_regular_file()) {
            all_files.emplace_back(entry);
          }
        }
      }

      unsigned int chunk = all_files.size() / num_workers_;

      for(std::size_t i = 0; i < num_workers_; ++i) {
        std::size_t start = i * chunk;
        std::size_t end = (i == num_workers_ - 1) ? all_files.size() : start + chunk;
        threads.emplace_back([this, start, end] {
          worker(start, end);
        });
      }

      for(auto& t : threads) {
        t.join();
      }
      
      return WorkerStats{fileCount_, allfileBytes_};

    }

    void worker(std::size_t start, std::size_t end) {
      for(unsigned int i = start; i < end; ++i) {
        fileCount_.fetch_add(1, std::memory_order_relaxed);
        allfileBytes_.fetch_add(all_files[i].file_size());
      }
    }

  private:
    std::filesystem::path root_path_;
    std::size_t num_workers_;
    std::vector<std::filesystem::directory_entry> all_files;
    std::vector<std::thread> threads;
    std::atomic<std::size_t> fileCount_;
    std::atomic<std::uintmax_t> allfileBytes_;

};

} // namespace lab0

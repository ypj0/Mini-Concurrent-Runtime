#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <thread>
#include <utility>
#include <vector>

#include "worker_stats.h"
#include "joining_thread.h"

namespace lab0 {
class FileScanner {
  public:
    FileScanner(std::filesystem::path root, std::size_t num_workers)
        : root_path_(std::move(root)), num_workers_(num_workers) {}

    WorkerStats scan() {
      namespace fs = std::filesystem;

      std::vector<fs::directory_entry> entries;
      for(const auto& entry : fs::recursive_directory_iterator(root_path_)) {
        if(entry.is_regular_file()) {
          entries.emplace_back(entry);
        }
      }

      WorkerStats stats;
      const auto files_count = entries.size();
      if(files_count == 0) {
        return stats;
      }

      auto each_file_cnt = files_count / num_workers_;
      if(files_count % num_workers_ != 0) { //有余数则前面的worker多拿一个
        each_file_cnt += 1;
      }
      const auto worker_count = (files_count + each_file_cnt -1) / each_file_cnt; // ceil(a/b) = (a + b - 1) / b 向上取整

      std::vector<WorkerStats> results(worker_count); // 不同worker写不同的槽位，无竞争

      {
        std::vector<JoiningThread> workers;
        workers.reserve(worker_count);
        for(std::size_t worker_id = 0; worker_id < worker_count; ++worker_id) {
          const auto strat = worker_id * each_file_cnt;
          const auto end = std::min(files_count, strat + each_file_cnt);
          workers.emplace_back([worker_id, strat, end, &entries, &results] {
            WorkerStats local;
            for(std::size_t index = strat; index < end; ++index) {
              local.files_scanned++;
              local.total_bytes += entries[index].file_size();
              local.ext_counts[entries[index].path().extension().string()]++;
            }
            results[worker_id] = std::move(local);
          });
        }
      }

      for(const auto& s : results) {
        stats += s;
      }

      return stats;

    }


  private:
    std::filesystem::path root_path_;
    std::size_t num_workers_;


};

} // namespace lab0

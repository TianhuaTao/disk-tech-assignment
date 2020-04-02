// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef ENGINE_RACE_ENGINE_RACE_H_
#define ENGINE_RACE_ENGINE_RACE_H_
#include <string>
#include "include/engine.h"
#include <unordered_map>
#include <iostream>
#include <pthread.h>
#include <functional>
namespace polar_race
{

struct Location
{
    size_t offset;
    size_t len;
};

class EngineRace : public Engine
{
public:
    static const size_t chunckSize = 128 * 1024 * 1024; // 128MB
    static const size_t thread_cnt = 8;

    static RetCode Open(const std::string &name, Engine **eptr);

    explicit EngineRace(const std::string &dir) 
    {
        for (size_t i = 0; i < thread_cnt; i++)
        {
            fd_index_tmp[i] = -1;
            pthread_mutex_init(mu_+i, NULL);
        }
    }

    ~EngineRace();

    RetCode Write(const PolarString &key,
                  const PolarString &value) override;

    RetCode Read(const PolarString &key,
                 std::string *value) override;

    /*
   * NOTICE: Implement 'Range' in quarter-final,
   *         you can skip it in preliminary.
   */
    RetCode Range(const PolarString &lower,
                  const PolarString &upper,
                  Visitor &visitor) override;

private:
    std::unordered_map<std::string, Location> index[thread_cnt];
    static int fd_index_tmp[thread_cnt];
    int fd_data[thread_cnt];
    Location nextLoc[thread_cnt];
    std::string dir_name;
    size_t read_data_file(size_t bucket_id, Location &loc, char *buf);
    static inline void log(std::string msg)
    {
        // std::cout << "[LOG] " << msg << std::endl;
    }
    RetCode read_index_file(int bucket_id, int &fd);
    char *mem_tmp[thread_cnt];
    size_t mem_offset[thread_cnt];
    size_t alloc_size_tmp[thread_cnt];
    pthread_mutex_t mu_[thread_cnt];

    size_t calculate_bucket_id(const std::string &key)
    {
        std::hash<std::string> hash_func;
        return hash_func(key) % thread_cnt;
    }
};

} // namespace polar_race

#endif // ENGINE_RACE_ENGINE_RACE_H_

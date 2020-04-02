// Copyright [2018] Alibaba Cloud All rights reserved
#include "engine_race.h"
#include "util.h"
#include <fcntl.h>
#include <iostream>
#include <map>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
namespace polar_race {

int EngineRace::fd_index_tmp[EngineRace::thread_cnt] = {};

RetCode Engine::Open(const std::string &name, Engine **eptr) {
    return EngineRace::Open(name, eptr);
}

Engine::~Engine() {
}

// read (key_size, key, location)
RetCode EngineRace::read_index_file(int bucket_id, int &fd) {
    ssize_t rd;
    int sz;
    char buf[1024];
    while ((rd = read(fd, (char *)&sz, 4)) > 0) { // key size is a 4-byte int
        if (rd != 4) {
            fprintf(stderr, "wrong index file, cannot read key size\n");
            return kIOError;
        }
        if (sz == 0) // no records any more
        {
            rd = 0;
            break;
        }
        // read key
        rd = read(fd, buf, sz);
        if (rd != sz) {
            fprintf(stderr, "wrong index file, cannot read key\n");
            return kIOError;
        }
        buf[sz] = 0;
        std::string key(buf);

        //read location
        Location pos;
        rd = read(fd, (char *)&(pos), sizeof(Location));
        if (rd != sizeof(Location)) {
            fprintf(stderr, "wrong index file, cannot read location\n");
            return kIOError;
        }
        this->index[bucket_id][key] = pos;
    }
    if (rd == 0) // end of file
        return kSucc;
    if (rd < 0)
        return kIOError;
    return kSucc;
}

// 1. Open engine
// check if index.tmp exists
// if yes, maybe a crash just happened, combine the index
// if no, load index
RetCode EngineRace::Open(const std::string &name, Engine **eptr) {
    *eptr = NULL;
    log("Opening file");
    EngineRace *engine_race = new EngineRace(name);
    engine_race->dir_name = name;
    if (!FileExists(name) && 0 != mkdir(name.c_str(), 0755)) {
        return kIOError;
    }
    // init every bucket
    for (size_t i = 0; i < thread_cnt; i++) {
        int fd_index;
        int fd_index_tmp = open((name + "/index_" + std::to_string(i) + ".tmp").c_str(), O_RDWR);
        if (fd_index_tmp < 0) {
            fd_index = open((name + "/index_" + std::to_string(i)).c_str(), O_RDWR);
            if (fd_index < 0) {
                // empty path
                // open new index.tmp file
                log("Path is empty");

                fd_index_tmp = open((name + "/index_" + std::to_string(i) + ".tmp").c_str(), O_RDWR | O_CREAT, 0644);
                if (fd_index_tmp < 0) {
                    delete engine_race;
                    return kIOError;
                }
                engine_race->fd_index_tmp[i] = fd_index_tmp;

                int fd_data = open((name + "/data_" + std::to_string(i)).c_str(), O_RDWR | O_CREAT, 0644);

                if (fd_data < 0) {
                    delete engine_race;
                    return kIOError;
                }
                engine_race->fd_data[i] = fd_data;
            } else {
                // last closing successful
                log("Data found");

                // read in index
                RetCode ret = engine_race->read_index_file(i, fd_index);
                if (ret != kSucc) {
                    log("read_index_file Error");
                    delete engine_race;
                    return ret;
                }
                // open previoud data file
                int fd_data = open((name + "/data_" + std::to_string(i)).c_str(), O_RDWR);
                if (fd_data < 0) {
                    delete engine_race;
                    return kIOError;
                }
                engine_race->fd_data[i] = fd_data;

                // open new index.tmp file
                fd_index_tmp = open((name + "/index_" + std::to_string(i) + ".tmp").c_str(), O_RDWR | O_CREAT, 0644);
                if (fd_index_tmp < 0) {
                    delete engine_race;
                    return kIOError;
                }
                engine_race->fd_index_tmp[i] = fd_index_tmp;
            }
        } else {
            // combine
            // index.tmp is found, so the engine did not close properly
            log("Engine was not closed properly last time");
            log("Fixing index...");
            fd_index = open((name + "/index_" + std::to_string(i)).c_str(), O_RDWR);
            if (fd_index < 0) {
                // no base index, only read index.tmp
                log("No base index found...");
                RetCode ret = engine_race->read_index_file(i, fd_index_tmp);
                if (ret != kSucc)
                    return ret;
                log("Read index.tmp OK");

                // open previoud data file
                int fd_data = open((name + "/data_" + std::to_string(i)).c_str(), O_RDWR);
                if (fd_data < 0) {
                    delete engine_race;
                    return kIOError;
                }
                engine_race->fd_data[i] = fd_data;
            } else {
                // update base index, first read index, then index.tmp
                log("Merging index...");
                log("Read base index file...");
                RetCode ret = engine_race->read_index_file(i, fd_index);
                if (ret != kSucc)
                    return ret;

                // read index.tmp
                log("Read index.tmp");
                ret = engine_race->read_index_file(i, fd_index_tmp);
                if (ret != kSucc)
                    return ret;

                // open previoud data file
                int fd_data = open((name + "/data_" + std::to_string(i)).c_str(), O_RDWR);
                if (fd_data < 0) {
                    delete engine_race;
                    return kIOError;
                }
                engine_race->fd_data[i] = fd_data;
            }
            // delete old index.tmp, create new index.tmp
            close(fd_index_tmp);
            remove((name + "/index_" + std::to_string(i) + ".tmp").c_str());
            fd_index_tmp = open((name + "/index_" + std::to_string(i) + ".tmp").c_str(), O_RDWR | O_CREAT, 0644);
            if (fd_index_tmp < 0) {
                delete engine_race;
                return kIOError;
            }
            engine_race->fd_index_tmp[i] = fd_index_tmp;
        }

        // allocate space for index.tmp file
        if (engine_race->fd_index_tmp[i] >= 0) {
            if (posix_fallocate(engine_race->fd_index_tmp[i], 0, chunckSize) != 0) {
                std::cerr << "posix_fallocate failed: " << strerror(errno) << std::endl;
                close(engine_race->fd_index_tmp[i]);
                return kIOError;
            }
        }

        // mmap
        void *ptr = mmap(NULL, chunckSize, PROT_READ | PROT_WRITE,
                         MAP_SHARED, engine_race->fd_index_tmp[i], 0);
        if (ptr == MAP_FAILED) {
            std::cerr << "MAP_FAILED: " << strerror(errno) << std::endl;
            close(engine_race->fd_index_tmp[i]);
            return kIOError;
        }
        // set 0 to all, or the engine dosen't know where to end when next load
        memset(ptr, 0, chunckSize);
        log(std::to_string(i) + ": mmap loaded ");
        engine_race->mem_tmp[i] = (char *)ptr;
        engine_race->mem_offset[i] = 0;
        engine_race->alloc_size_tmp[i] = chunckSize;
    }
    *eptr = engine_race;
    return kSucc;
}

// 2. Close engine
// write index hash map to file, delete the temporary index file
// because the temporary index file is append-only, contains obsolete
// index, so next load operation can be faster.
EngineRace::~EngineRace() {
    log("Saving index");
    for (size_t i = 0; i < thread_cnt; i++) {
        munmap(mem_tmp[i], alloc_size_tmp[i]);
        int fd_index_save = open((dir_name + "/index_" + std::to_string(i)).c_str(), O_RDWR | O_CREAT, 0644);
        for (auto &it : index[i]) {
            const std::string &key = it.first;
            const Location &loc = it.second;
            int s = key.size();
            FileAppend(fd_index_save, (char *)&s, sizeof(s));     //write key length
            FileAppend(fd_index_save, key);                       // write key content
            FileAppend(fd_index_save, (char *)&loc, sizeof(loc)); // write location
        }
        close(fd_index_save);

        fsync(fd_index_tmp[i]); // wait, or can't be removed
        close(fd_index_tmp[i]);
        remove((dir_name + "/index_" + std::to_string(i) + ".tmp").c_str()); // might fail if no fsync
    }

    log("Write to index file");
    log("Remove index.tmp");
    log("Engine closed");
}

// 3. Write a key-value pair into engine
RetCode EngineRace::Write(const PolarString &key, const PolarString &value) {
    // static int write_cnt = 0;
    size_t bucket_id = calculate_bucket_id(key.ToString());
    pthread_mutex_lock(mu_ + bucket_id);

    nextLoc[bucket_id].len = value.size();
    index[bucket_id][key.ToString()] = nextLoc[bucket_id]; // update in-memory hash

    FileAppend(fd_data[bucket_id], value.ToString()); // write value data

    // write index.tmp
    int s = key.size();

    // direct IO to write index.tmp, cannot flush to disk if killed with sig-9
    // if (FileAppend(fd_index_tmp, (char *)&s, sizeof(s))) //write key length
    //   log("Cannot write key size");
    // if (FileAppend(fd_index_tmp, key.ToString())) // write key content
    //   log("Cannot write key");
    // if (FileAppend(fd_index_tmp, (char *)&nextLoc, sizeof(nextLoc))) // write location
    //   log("Cannot write location");
    // fsync(fd_index_tmp);

    // mmap way to write index.tmp, can write to disk even if this process is killed
    // as long as kernel is good
    memcpy(mem_tmp[bucket_id] + mem_offset[bucket_id], (char *)&s, sizeof(s));
    mem_offset[bucket_id] += sizeof(s);

    memcpy(mem_tmp[bucket_id] + mem_offset[bucket_id], key.data(), key.size());
    mem_offset[bucket_id] += key.size();

    memcpy(mem_tmp[bucket_id] + mem_offset[bucket_id], (char *)&(nextLoc[bucket_id]), sizeof(nextLoc[bucket_id]));
    mem_offset[bucket_id] += sizeof(nextLoc[bucket_id]);

    nextLoc[bucket_id].offset += nextLoc[bucket_id].len;
    pthread_mutex_unlock(mu_ + bucket_id);
    return kSucc;
}

// 4. Read value of a key
RetCode EngineRace::Read(const PolarString &key, std::string *value) {
    auto bucket_id = calculate_bucket_id(key.ToString());
    pthread_mutex_lock(mu_ + bucket_id);
    auto it = index[bucket_id].find(key.ToString());
    if (it == index[bucket_id].end()) {
        pthread_mutex_unlock(mu_ + bucket_id);
        return kNotFound;
    }
    char read_buf[4097];
    Location &loc = it->second;
    read_data_file(bucket_id, loc, read_buf);
    read_buf[loc.len] = 0;
    *value = std::string(read_buf);
    pthread_mutex_unlock(mu_ + bucket_id);
    return kSucc;
}
// seek and read
size_t EngineRace::read_data_file(size_t bucket_id, Location &loc, char *buf) {
    lseek(fd_data[bucket_id], loc.offset, SEEK_SET);
    size_t rd = read(fd_data[bucket_id], buf, loc.len);
    lseek(fd_data[bucket_id], 0, SEEK_END);
    return rd;
}
/*
 * NOTICE: Implement 'Range' in quarter-final,
 *         you can skip it in preliminary.
 */
// 5. Applies the given Vistor::Visit function to the result
// of every key-value pair in the key range [first, last),
// in order
// lower=="" is treated as a key before all keys in the database.
// upper=="" is treated as a key after all keys in the database.
// Therefore the following call will traverse the entire database:
//   Range("", "", visitor)
RetCode EngineRace::Range(const PolarString &lower, const PolarString &upper,
                          Visitor &visitor) {
    return kSucc;
}

} // namespace polar_race

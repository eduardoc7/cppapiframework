#pragma once
#include "GenericQueue.hpp"
#include <queue>
#include <unordered_map>

class StdQueue : public GenericQueue {
    std::unordered_map<std::string, std::queue<std::string>> queue_map;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        persistentdata;

  public:
    void push(const std::string &queue, const std::string &data) override;
    void pushToLater(const std::string &queue, const std::string &data,
                     std::chrono::system_clock::time_point tp) override;

    auto pop(const std::string &queue, int timeout)
        -> std::optional<std::string> override;

    auto getName() const -> std::string override;
    void setName(const std::string &name) override;

    auto getNumQueues() const -> size_t { return queue_map.size(); }
    auto getQueueSize(const std::string &queue) const -> size_t {
        return queue_map.at(queue).size();
    }

    auto getPersistentData(const std::string &name) const
        -> std::unordered_map<std::string, std::string> override {
        return persistentdata.at(name);
    }

    void setPersistentData(
        const std::string &name,
        const std::unordered_map<std::string, std::string> &data) override {
        auto &map = persistentdata[name];
        for (auto &d : data) {
            map.insert_or_assign(d.first, d.second);
        }
    }

    void delPersistentData(const std::string &name) override {
        persistentdata.erase(name);
    }

    auto isConnected() const -> bool override { return true; }

    StdQueue();
    ~StdQueue() override;
};

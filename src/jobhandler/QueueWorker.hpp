#pragma once

#include "../queues/GenericQueue.hpp"
#include "../stdafx.hpp"
#include "../utils/ProcessHelper.hpp"
#include "../utils/ScopedStreamRedirect.hpp"
#include "JobsHandler.hpp"
#include <fstream>
#include <unistd.h>
#include <utility>

namespace job {

class QueueWorker {
  protected:
    std::shared_ptr<JobsHandler> jobhandler;
    std::shared_ptr<GenericQueue> queueServiceInst;
    std::shared_ptr<ProcessHelper> processHelperInst;

    int queueTimeout{1}, retryInTimeout{0};
    int64_t jobLogExpireSeconds{3600};

    std::atomic<bool> running{true};
    std::atomic<bool> forkToHandle{false};
    std::atomic<bool> cleanSuccessfulJobsLogs{true};

    static auto allocateJobOutputStream(const Poco::JSON::Object::Ptr &json)
        -> std::pair<std::fstream, std::fstream>;

  public:
    void setProcessHelper(std::shared_ptr<ProcessHelper> pHelper) {
        processHelperInst = std::move(pHelper);
    }

    auto getProcessHelper() -> std::shared_ptr<ProcessHelper> {
        if (!processHelperInst) {
            processHelperInst = std::make_shared<ProcessHelper>();
        }

        return processHelperInst;
    }

    virtual auto
    process_retry_condition(const std::shared_ptr<QueueableJob> &job)
        -> jobStatus;

    auto handle_job_run(const std::shared_ptr<QueueableJob> &newjob,
                        const Poco::JSON::Object::Ptr &json,
                        GenericQueue::datamap_t &datamap) -> jobStatus;

    /**
     * @brief instanciates and run the job
     * @todo improve the error catching design, log the tries
     *
     * param queue queue name
     * @param datamap all job data
     * @return jobStatus job result
     */
    auto work(const std::string & /*queue*/, GenericQueue::datamap_t &datamap)
        -> jobStatus;

    auto fork_process() -> pid_t;

    /**
     * @brief Adds a job to the queue
     *
     * @tparam T job type
     * @param queue queue name
     * @param job job instance
     * @return std::string job uuid
     */
    template <class T>
    auto push(const std::string &queue, const T &job) -> std::string {
        auto json = jobhandler->create_jobpayload(job);
        constexpr size_t KEYSIZE = sizeof("job_instance:") + 36;
        std::string jobuuid =
            Poco::UUIDGenerator::defaultGenerator().createOne().toString();

        std::string persistentkey;
        persistentkey.reserve(KEYSIZE);
        persistentkey = "job_instance:";
        persistentkey += jobuuid;

        json->set("uuid", jobuuid);

        std::stringstream sstr;
        json->stringify(sstr);

        queueServiceInst->setPersistentData(
            persistentkey, {{"tries", "0"},
                            {"maxtries", std::to_string(job.getMaxTries())},
                            {"payload", sstr.str()},
                            {"created_at_unixt", std::to_string(time(nullptr))},
                            {"className", std::string(job.getName())}});

        queueServiceInst->push(queue, persistentkey);

        return jobuuid;
    }

    // void push(const std::string &queue, const Poco::JSON::Object::Ptr &json);
    auto pop(const std::string &queue, int timeout) -> std::string;

    /**
     * @brief Process the job result, re-adds in the queue if is errorretry
     *
     * @param queue queue name
     * @param jobname name of the job to the queue
     * @param workresult job run result
     */
    void process_job_result(
        const std::string &queue, const std::string &jobname,
        const std::unordered_map<std::string, std::string> &datamap,
        jobStatus workresult) {
        switch (workresult) {
        case noerror:
            if (cleanSuccessfulJobsLogs) {
                queueServiceInst->delPersistentData(jobname);
            } else {
                queueServiceInst->setPersistentData(jobname, datamap);
                queueServiceInst->expire(jobname, jobLogExpireSeconds);
            }
            break;

        case errorremove:
            queueServiceInst->setPersistentData(jobname, datamap);
            queueServiceInst->expire(jobname, jobLogExpireSeconds);
            break;

        case errexcept:
        case errorretry:
            queueServiceInst->setPersistentData(jobname, datamap);
            queueServiceInst->push(queue, jobname);
            break;
        }
    }

    auto do_one(const std::string &queue, std::string &jobname) -> bool;

    /**
     * @brief Pop job from the queue and run it
     *
     * @param queue queue name
     * @return true has job in the queue and runned it
     * @return false queue empty
     */
    auto do_one(const std::string &queue) -> bool;

    /**
     * @brief Run jobs in loop until running is false
     *
     * @param queue queue name
     */
    void loop(const std::string &queue) {
        while (running) {
            do_one(queue);
        }
    }

    void setForkToHandleJob(bool forkstatus) { forkToHandle = forkstatus; }

    /**
     * @brief Should keep the completed jobs data in the persistent storage?
     *
     * @param value true keep the logs; false don't
     */
    void setCleanSuccessfulJobsLogs(bool value) {
        cleanSuccessfulJobsLogs = value;
    }

    /**
     * @brief Finished jobs should be keep in the persistent storage for how
     * many seconds after it's finished?
     *
     * @param value seconds in integer
     */
    void setJobFinishedExpireSeconds(int64_t value) {
        jobLogExpireSeconds = value;
    }

    virtual auto operator=(const QueueWorker &) -> QueueWorker & = delete;
    virtual auto operator=(QueueWorker &&) -> QueueWorker & = delete;

    QueueWorker(const QueueWorker &) = delete;
    QueueWorker(QueueWorker &&) = delete;

    QueueWorker(std::shared_ptr<JobsHandler> jobh,
                std::shared_ptr<GenericQueue> queueService);
    virtual ~QueueWorker();
};

} // namespace job

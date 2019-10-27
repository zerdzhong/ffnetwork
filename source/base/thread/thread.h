//
// Created by zerdzhong on 2019/10/13.
//

#ifndef FFBASE_THREAD_H
#define FFBASE_THREAD_H

#include <string>
#include <atomic>
#include <memory>
#include <thread>

#include "messageloop/task_runner.h"

namespace ffbase {

class Thread {
public:
    explicit Thread(const std::string& name = "");
    ~Thread();

    void Start(closure thread_func = nullptr);
    
    std::shared_ptr<TaskRunner> GetTaskRunner() const;
    void Join();
    
    static void SetCurrentThreadName(const std::string& name);
    std::string GetName() const;
    
private:
    std::unique_ptr<std::thread> thread_;
    std::shared_ptr<TaskRunner> task_runner_;
    std::atomic_bool joined_;
    std::string name_;
};

}//end of namespace ffbase


#endif //FFBASE_THREAD_H

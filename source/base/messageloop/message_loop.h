//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_MESSAGE_LOOP_H
#define FFBASE_MESSAGE_LOOP_H

#include "macros.h"
#include <cstdint>
#include <memory>
#include <functional>

namespace ffbase {

class TaskRunner;
class MessageLoopImpl;

class MessageLoop {
public:
    static MessageLoop& GetCurrent();
    
    void Run();
    void Terminate();
    void AddTaskObserver(intptr_t key, std::function<void()> callback);
    void RemoveObserver(intptr_t key);
    
    std::shared_ptr<TaskRunner> GetTaskRunner() const;
    
    static void EnsureInitializedForCurrentThread();
    static bool IsInitializedForCurrentThread();

    ~MessageLoop();
    
private:
    std::shared_ptr<TaskRunner> task_runner_;
    std::shared_ptr<MessageLoopImpl> loop_;
    
    MessageLoop();
    
    std::shared_ptr<MessageLoopImpl> GetLoopImpl() const;
    
    FF_DISALLOW_COPY_AND_ASSIGN(MessageLoop);
    
};

}

#endif //FFNETWORK_MESSAGE_LOOP_H

//
// Created by carlo on 7/11/2024.
//

#include <condition_variable>
#include <iostream>
#include "AssetThreat.h"

namespace VULKAN{
    AssetThreat::AssetThreat() {
        running = true;
        assetThreat = std::thread(&AssetThreat::Run, this);

    }

    void AssetThreat::Run() {
        while (running){
            std::cout<<"Running\n";
            std::function<void()>task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                conditionVariable.wait(lock, [this, task](){return !taskQueue.empty() || !running;});
                if (taskQueue.empty() || !running){
                    std::cout<<"TaskQueue was empty and it shouldn't be!\n";
                    return;
                }
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
            task();
            
        }
    }

    void AssetThreat::AddTask(std::function<void()> task) {
        {
            std::lock_guard<std::mutex>lockGuard (queueMutex);
            taskQueue.push(task);
        }
        conditionVariable.notify_one();
    }
}

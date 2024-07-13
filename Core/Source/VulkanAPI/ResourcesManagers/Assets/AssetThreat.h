//
// Created by carlo on 7/11/2024.
//

#ifndef EDITOR_ASSETTHREAT_H
#define EDITOR_ASSETTHREAT_H

#include <thread>
#include <atomic>
#include <queue>
#include <functional>

namespace VULKAN {

    class AssetThreat {

    public:
        AssetThreat();
        void AddTask(std::function<void()> task);

    private:
        void Run();
        std::atomic<bool> running;
        std::queue<std::function<void()>> taskQueue;
        std::thread assetThreat;
        std::mutex queueMutex;
        std::condition_variable conditionVariable;
        
    };
    


}
#endif //EDITOR_ASSETTHREAT_H

#include "workpool.h"

Workpool Workpool::instance;

Workpool::~Workpool()
{
    running = false;
    for (auto& thread : threads) {
        thread.join();
    }
}

void Workpool::init(int maxWorkers)
{
    instance.running = true;
    for (int i = 0; i < maxWorkers; i++) {
        instance.threads.push_back(std::thread(std::bind(&Workpool::worker,&instance)));
    }
}

void Workpool::tick()
{
    Task task;
    {
        std::lock_guard<std::mutex> lock(completeLock);
        if (complete.empty()) {
            return;
        }

        task = complete.back();
        complete.pop();
    }
    if (task.complete) {
        task.complete();
    }
}

void Workpool::addTask(TaskFunction task, CompleteFunction complete)
{
    std::lock_guard<std::mutex> lock(scheduleLock);
    schedule.push({ task, complete });
}

void Workpool::worker()
{
    while (running) {
        Task task;
        {
            std::lock_guard<std::mutex> lock(scheduleLock);
            if (schedule.empty()) {
                continue;
            }
            task = schedule.back();
            schedule.pop();
        }
        if (task.task) {
            task.task();
        }
        {
            std::lock_guard<std::mutex> lock(completeLock);
            complete.push(task);
        }
    }
}

#ifndef _workpool_h
#define _workpook_h

#include <mutex>
#include <queue>
#include <thread>
#include <functional>

class Workpool
{
public:
    ~Workpool();
    using TaskFunction = std::function<void(void)>;
    using CompleteFunction = std::function<void(void)>;

    static Workpool instance;
    static void init(int maxWorkers=4);

    void tick();
    void addTask(TaskFunction task, CompleteFunction complete);

private:
    void worker();

    struct Task {
        TaskFunction task;
        CompleteFunction complete;
    };

    std::vector<std::thread> threads;
    std::mutex scheduleLock;
    std::queue<Task> schedule;
    std::mutex completeLock;
    std::queue<Task> complete;

    bool running = false;
};

#endif _workpool_h
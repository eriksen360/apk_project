#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

/*
TODO: Add rule of 5
*/

template <typename T>
class MessageQueue
{
public:
    MessageQueue() {}

    void enqueue(const T &item)
    {
        std::scoped_lock<std::mutex> lock(mtx); // lock the mutex for this scope
        queue.push(item);
        cond_var.notify_one();
    }

    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mtx); // use unique_lock for wait() and limited to this scope
        cond_var.wait(lock, [this]
                      { return !queue.empty(); }); // make sure the queue is not empty
        T value = queue.front();
        queue.pop();

        return value;
    }

private:
    std::queue<T> queue;
    mutable std::mutex mtx; // mutable allows the mutex to be used in const functions
    std::condition_variable cond_var;
};
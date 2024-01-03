#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "Event.hpp"
#include <concepts>
#include <type_traits>
#include <memory>

/*
TODO: Add rule of 5
*/
namespace bank{

template <typename T>
concept DerivedFromEvent = std::is_base_of<Event, T>::value;

template <typename T>
concept UniquePtrToDerivedFromEvent =
    requires(T t) {
        {
            t.get()
        } -> std::convertible_to<Event *>;
    } && std::is_same_v<T, std::unique_ptr<typename T::element_type>> && DerivedFromEvent<typename T::element_type>;

template <typename T>
    requires UniquePtrToDerivedFromEvent<T>
class EventMessageQueue
{
public:
    EventMessageQueue()
    {

        // constexpr to ensure that T derives from Event
    }

    void enqueue(const T &item)
    {
        std::scoped_lock<std::mutex> lock(mtx); // lock the mutex for this scope
        queue.push(item);
        cond_var.notify_one();
    }

    void enqueue(T &&item) // Used for movable
    {
        std::scoped_lock<std::mutex> lock(mtx);
        queue.push(std::move(item));
        cond_var.notify_one();
    }

    bool frontIsOfType(const std::type_info &type)
    {
        std::scoped_lock<std::mutex> lock(mtx);
        if (!queue.empty())
        {
            return typeid(*queue.front()) == type;
        }
        return false;
    }

    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cond_var.wait(lock, [this]
                      { return !queue.empty(); });
        T value = std::move(queue.front()); // Use std::move for movable types
        queue.pop();

        return value;
    }

    size_t size()
    {
        std::scoped_lock<std::mutex> lock(mtx);
        return queue.size();
    }

    ~EventMessageQueue(){};

private:
    std::queue<T> queue;
    mutable std::mutex mtx; // mutable allows the mutex to be used in const functions
    std::condition_variable cond_var;
};
}
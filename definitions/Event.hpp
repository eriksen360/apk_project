#pragma once
#include <chrono>

class Event  // Event should also have a move constructor since data is moved from a client to the queue
{
public:
    Event() : id(-1), createdAt(std::chrono::system_clock::now()) {}

    virtual void createID() = 0;

    std::chrono::_V2::system_clock::time_point getCreatedAt() const
    {
        return createdAt;
    };

    int getEventId() const
    {
        return id;
    };

    virtual ~Event() {}

protected:
    void setID(int new_id)
    {
        id = new_id;
    }

private:
    const std::chrono::_V2::system_clock::time_point createdAt;
    int id; // TODO: Use boost library UUID as Id
};

#pragma once
#include <chrono>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

class Event  // Event should also have a move constructor since data is moved from a client to the queue
{
public:
    Event() : createdAt(std::chrono::system_clock::now()) {
        boost::uuids::random_generator gen;
        boost::uuids::uuid _id = gen();
        id = _id;
    }

    std::chrono::_V2::system_clock::time_point getCreatedAt() const
    {
        return createdAt;
    };

    boost::uuids::uuid getId() const
    {
        return id;
    };

    virtual ~Event() {}

private:
    const boost::uuids::uuid id;
    const std::chrono::_V2::system_clock::time_point createdAt;
};

/**
 * @file Event.hpp
 *
 * @author Filip Møgelvang Hansen & Mathias Fenger-Eriksen
 *
 * @brief File containing the base class Event.
 *
 */
#pragma once
#include <chrono>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/**
 * @brief Base class for all events to me queued
 *
 */
class Event
{
public:
    Event() : createdAt(std::chrono::system_clock::now())
    {
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
    boost::uuids::uuid id;
    std::chrono::_V2::system_clock::time_point createdAt;
};

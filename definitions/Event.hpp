#include <chrono>

class Event
{
public:
    Event() : id(-1), createdAt(std::chrono::system_clock::now()) {}

    virtual void createID() = 0;

    std::chrono::_V2::system_clock::time_point getCreatedAt()
    {
        return createdAt;
    };

    virtual int getEventId()
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

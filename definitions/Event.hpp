

class Event {
private:
    int createdAt;
public:
    Event() {};
    int getCreatedAt() {
        return createdAt;
    };
    virtual int getEventId();
    virtual ~Event();
};


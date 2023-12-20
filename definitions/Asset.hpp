#include <string>
class Asset {
private:
    int id;
public:
    virtual int foo() = 0;
    Asset() {};
    virtual ~Asset() {};
};


class Cash {};

class Security : Asset {
public:
    Security() {};
    virtual ~Security() {};
};

class Bond : Security {
public:
    int foo() { return 0; };
    Bond() {};
    ~Bond() {};
};

class Stock : Security {
public:
    int foo() { return 0; };
    Stock() {};
    ~Stock() {};
};

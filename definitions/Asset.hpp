#include <string>

class Asset {
private:
    int id;
public:
    virtual int foo() = 0;
    Asset() {};
    virtual ~Asset() {};
};

class Security : public Asset {
public:
    Security() {};
    virtual ~Security() {};
};

class Bond : public Security {
public:
    int foo() { return 0; };
    Bond() {};
    ~Bond() {};
};

class Stock : public Security {
public:
    int foo() { return 0; };
    Stock() {};
    ~Stock() {};
};

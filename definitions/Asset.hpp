
class Asset {
public:
    virtual int foo() = 0;
    Asset() {};
    virtual ~Asset() {};
};

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

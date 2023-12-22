#include <string>

enum Currency
{
    DKK = 0,
    USD = 1
};

class Asset {  //TODO: Asset should have a move constructor with noexcept since it should be moved between accounts
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

class Cash : public Asset {
private:
    int foo() { return 0; };
    int amount = 0;
    const Currency currency = DKK;
public:
    Cash(int amount, Currency currency) : amount(amount), currency(currency) {};

    int getAmount() const {
        return amount;
    }

    void addAmount(int n){
        amount += n;
    }

    void removeAmount(int n){
        amount -= n;
    }

    Currency getCurrency() const {
        return currency;
    }

    // Add move constructor?

};

/*
template<typename T, typename U>
struct is_convertible<>  {

    //T og U er ikke hvis T er Stock, Bond og skal til Bond, Stock
}

template<typename T, typename U>
struct is_convertible<Cash, Stock>  {
    return true;
}

template<typename T>
struct is_convertible<Cash, Cash>  {


    // bestemte valutaer kan ikke konverteres fra og til hinanden
}

*/
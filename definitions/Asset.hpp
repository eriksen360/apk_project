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
    Asset() {};
    virtual ~Asset() {};
};

const class Security : public Asset {
public:
    Security() {};
    virtual ~Security() {};
    virtual void print() const = 0;
};

const class Bond : public Security {
public:
    Bond() {};
    ~Bond() {};
    void print() const {
        std::cout << "Bond" << std::endl;
    }
};

const static std::unordered_map<std::string, Stock> availableStocks = {
    {"AMD", Stock()},
    {"AAPL", Stock()},
    {"GOOG", Stock()},
    {"MSFT", Stock()}
};

const class Stock : public Security {
public:
    Stock() {};
    ~Stock() {};
    void print() const {
        std::cout << "Stock" << std::endl;
    }
};

const static std::unordered_map<std::string, Bond> availableBonds = {
    {"10Y US Treasury", Bond()},
    {"30Y US Treasury", Bond()},
    {"10Y Danish Treasury", Bond()},
    {"30Y Danish Treasury", Bond()}
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
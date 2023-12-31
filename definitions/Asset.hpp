#include <string>
#include <chrono>

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
private:
    int assetId;
    std::string name;
    double buyPrice;
    double currentPrice;
    double sellPrice;
    const std::chrono::_V2::system_clock::time_point purchasedAt;
    const std::chrono::_V2::system_clock::time_point soldAt; 
public:
    Security() {};
    virtual ~Security() {};
    virtual void print() const = 0;
    void getBuyPrice() const {
        return buyPrice;
    }
    void getCurrentPrice() const {
        return currentPrice;
    }
    void getSellPrice() const {
        return sellPrice;
    }
    virtual void buySignal() const {
        purchasedAt = std::chrono::system_clock::now();
    }
    virtual void sellSignal() const {
        soldAt = std::chrono::system_clock::now();
    }
};

const class Bond : public Security {
public:
    Bond(std::string name, double buyPrice, double currentPrice, double sellPrice) 
     : Security(name, buyPrice, currentPrice, sellPrice) {};
    ~Bond() {};
    void print() const {
        std::cout << "Bond" << std::endl;
        std::cout << "Purchase price: " << this->getBuyPrice() << std::endl;
        std::cout << "Current price: " << this->getCurrentPrice() << std::endl;
        std::cout << "Selling price: " << this->getSellPrice() << std::endl;
    }
};

const static std::unordered_map<std::string, Stock> availableStocks = {
    {"AMD", Stock("AMD", 10.0, 10.0, 10.0)},
    {"AAPL", Stock("AAPL", 9.0, 9.0, 9.0)},
    {"TSLA", Stock("TSLA", 8.0, 8.0, 8.0)},
    {"GOOG", Stock("GOOG", 7.0, 7.0, 7.0)},
    {"MSFT", Stock("MSFT", 6.0, 6.0, 6.0)}
};

const class Stock : public Security {
public:
    Stock(std::string name, double buyPrice, double currentPrice, double sellPrice) 
     : Security(name, buyPrice, currentPrice, sellPrice) {};
    ~Stock() {};
    void print() const {
        std::cout << "Stock" << std::endl;
        std::cout << "Purchase price: " << this->getBuyPrice() << std::endl;
        std::cout << "Current price: " << this->getCurrentPrice() << std::endl;
        std::cout << "Selling price: " << this->getSellPrice() << std::endl;
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
#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/*
All assets have move constructors and move assignment operator, 
as they should be moveable, not copyable. 

*/

enum Currency
{
    DKK = 0,
    USD = 1
};

class Asset
{ 
public:
    Asset(){};                                     // default constructor
    Asset(Asset &&) noexcept = default;            // move constructor
    Asset &operator=(Asset &&) noexcept = default; // move assignemnt
    virtual ~Asset() = default;
};

class Security : public Asset
{
private:
    boost::uuids::uuid id;
    std::string name;
    double buyPrice;
    double currentPrice;
    double sellPrice;
    std::chrono::_V2::system_clock::time_point purchasedAt;
    std::chrono::_V2::system_clock::time_point soldAt;

public:
    Security(std::string name, double buyPrice, double currentPrice, double sellPrice)
        : name(name), buyPrice(buyPrice), currentPrice(currentPrice), sellPrice(sellPrice)
    {
        boost::uuids::random_generator gen;
        boost::uuids::uuid _id = gen();
        id = _id;
    };

    Security(Security &&) noexcept = default;
    Security &operator=(Security &&) noexcept = default;
    virtual ~Security() = default;

    virtual void print() const = 0;
    std::string getName() const
    {
        return name;
    }
    double getBuyPrice() const
    {
        return buyPrice;
    }
    double getCurrentPrice() const
    {
        return currentPrice;
    }
    double getSellPrice() const
    {
        return sellPrice;
    }
    void buySignal()
    {
        purchasedAt = std::chrono::system_clock::now();
    }
    void sellSignal()
    {
        soldAt = std::chrono::system_clock::now();
    }
    boost::uuids::uuid getId() const
    {
        return id;
    }
};

class Bond : public Security
{
public:
    Bond(std::string name, double buyPrice, double currentPrice, double sellPrice)
        : Security(name, buyPrice, currentPrice, sellPrice){};

    Bond(Bond &&) noexcept = default;
    Bond &operator=(Bond &&) noexcept = default;

    ~Bond() = default;
    void print() const
    {
        std::cout << "Bond" << std::endl;
        std::cout << "Name: " << this->getName() << std::endl;
        std::cout << "Purchase price: " << this->getBuyPrice() << std::endl;
        std::cout << "Current price: " << this->getCurrentPrice() << std::endl;
        std::cout << "Selling price: " << this->getSellPrice() << std::endl;
    }
};

class Stock : public Security
{
public:
    Stock(std::string name, double buyPrice, double currentPrice, double sellPrice)
        : Security(name, buyPrice, currentPrice, sellPrice){};

    Stock(Stock &&) noexcept = default;
    Stock &operator=(Stock &&) noexcept = default;

    ~Stock() = default;
    void print() const
    {
        std::cout << "Stock" << std::endl;
        std::cout << "Name: " << this->getName() << std::endl;
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
    {"MSFT", Stock("MSFT", 6.0, 6.0, 6.0)}};

const static std::unordered_map<std::string, Bond> availableBonds = {
    {"10Y US Treasury", Bond("10Y US Treasury", 100.0, 100.0, 100.0)},
    {"30Y US Treasury", Bond("30Y US Treasury", 100.0, 100.0, 100.0)},
    {"10Y Danish Treasury", Bond("10Y Danish Treasury", 100.0, 100.0, 100.0)},
    {"30Y Danish Treasury", Bond("10Y Danish Treasury", 100.0, 100.0, 100.0)}};

class Cash : public Asset
{
private:
    int amount = 0;
    const Currency currency = DKK;

public:
    Cash(int amount, Currency currency) : amount(amount), currency(currency){};
    Cash(Cash &&) noexcept = default;
    Cash &operator=(Cash &&) noexcept = default;

    ~Cash() = default;

    int getAmount() const
    {
        return amount;
    }

    void addAmount(int n) noexcept
    {
        amount += n;
    }

    void removeAmount(int n) noexcept
    {
        amount -= n;
    }

    Currency getCurrency() const
    {
        return currency;
    }
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
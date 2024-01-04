#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/*
All assets have move/copy constructors and move/copy assignment operator. 
In reality it should only be moveable, to make sure assets are not copied. 
But for simplicity of the program, it is copyable. 
*/

enum Currency
{
    DKK = 0,
    USD = 1
};

class Asset
{ // TODO: Asset should have a move constructor with noexcept since it should be moved between accounts
private:
    boost::uuids::uuid id;

public:
    Asset()
    {
        boost::uuids::random_generator gen;
        boost::uuids::uuid _id = gen();
        id = _id;
    };
    boost::uuids::uuid getID() const
    {
        return id;
    }

    Asset(const Asset &) = default;
    Asset &operator=(const Asset &) = default;
    Asset(Asset &&) noexcept = default;
    Asset &operator=(Asset &&) noexcept = default;
    virtual ~Asset() = default;
};

class Security : public Asset
{
private:
    std::string name;
    double buyPrice;
    double currentPrice;
    double sellPrice;
    std::chrono::_V2::system_clock::time_point purchasedAt;
    std::chrono::_V2::system_clock::time_point soldAt;

public:
    Security(std::string name, double buyPrice, double currentPrice, double sellPrice)
        : name(name), buyPrice(buyPrice), currentPrice(currentPrice), sellPrice(sellPrice){};

    Security(const Security &) = default;
    Security &operator=(const Security &) = default;
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
};

class Bond : public Security
{
public:
    Bond(std::string name, double buyPrice, double currentPrice, double sellPrice)
        : Security(name, buyPrice, currentPrice, sellPrice){};

    Bond(const Bond &) = default;
    Bond &operator=(const Bond &) = default;
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

    Stock(const Stock &) = default;
    Stock &operator=(const Stock &) = default;
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

    Cash(const Cash &) = default;
    Cash &operator=(const Cash &) = default;
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

    void reduceAmount(int n) noexcept
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
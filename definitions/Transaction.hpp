#pragma once
#include "Event.hpp"
#include "Asset.hpp"
#include <tuple>
#include <set>
#include <concepts>
#include <type_traits>

enum TransactionType
{
    BUY = 0,
    SELL = 1,
    CONVERT = 2
};

class Transaction : public Event
{
public:
    Transaction(std::string to, std::string from) : Event(), toAccountEmail(to), fromAccountEmail(from){};
    Transaction(std::string to) : Event(), toAccountEmail(to){};
    virtual ~Transaction(){};

    std::string getToAccount()
    {
        return toAccountEmail;
    }

    std::string getFromAccount()
    {
        return fromAccountEmail;
    }
    virtual void print() const = 0; 

private:
    std::string toAccountEmail;
    std::string fromAccountEmail;
};

class CashTransaction : public Transaction
{
public:
    CashTransaction(int amount, std::string to, std::string from) : Transaction(to, from), amount(amount) {}
    CashTransaction(int amount, std::string to) : Transaction(to), amount(amount) {}
    int getTransactionAmount() { return amount; };

    int getAmount() const
    {
        return amount;
    }
    void print() const override
    {
        std::cout << "Transaction amount: " << amount << std::endl;
    }

private:
    int amount;
};

template <typename T>
    requires DerivedFromSecurity<T>
class SecurityTransaction : public Transaction
{
private:
    std::vector<T> securities;
    TransactionType type;

public:
    SecurityTransaction(std::string toEmail, T security, int amount, TransactionType _type) : Transaction(toEmail), type(_type)
    {
        for (int i = 0; i < amount; i++)
        {
            securities.push_back(security);
        }
    };
    
    double getTransactionBuyAmount() const {
        if (securities.size() > 0) {
            return securities[0].getBuyPrice() * securities.size();
        }
        return 0.0;
    }

    double getTransactionCurrentAmount() const {
        if (securities.size() > 0) {
            return securities[0].getCurrentPrice() * securities.size();
        }
        return 0.0; 
    }

    double getTransactionSellAmount() const {
        if (securities.size() > 0) {
            return securities[0].getSellPrice() * securities.size();
        }
        return 0.0; 
    }

    TransactionType getType() const
    {
        return type;
    }

    std::tuple<std::string, int> getSecuritiesOverview() {
        std::set <std::string> stockTypes;
        for (auto& security : securities) {
            stockTypes.insert(security.getName());
        }
        if (stockTypes.size() > 1) {
                throw std::logic_error("Stocks in transaction of different types.");
        }

        return std::tuple<std::string, int>(*stockTypes.begin(), securities.size());
    }

    // These methdos for adding and removing should happen for the account!!
    std::vector<T> getSecurities() const {
        return securities;
    }

    constexpr bool is_stock()
    {
        std::cout << "IS_STOCK=" << std::is_same<T, Stock>::value << std::endl;
        return std::is_same<T, Stock>::value;
    }

    constexpr bool is_bond()
    {
        return std::is_same<T, Bond>::value;
    }

    void print() const override
    {
        std::cout << "Id: " << getId() << std::endl;
        std::cout << "Type: " << type << std::endl;
        std::cout << "Number: " << securities.size() << std::endl;
        std::cout << "Type: " << getType() << std::endl;
    }

    ~SecurityTransaction() {}
};




// class BondTransaction : public SecurityTransaction
// {
// private:
//     std::vector<Bond> bonds;
// public:
//     BondTransaction(std::string toEmail, Bond bond, int amount, TransactionType _type)
//         : SecurityTransaction(toEmail, _type) {
//             for (int i = 0; i < amount; i++) {
//                 stocks.push_back(bond);
//             }
//         }

//     size_t getSize() const
//     {
//         return bonds.size();
//     }

//     double getTransactionCost() const {
//         if getSize() > 0 {
//             return bonds[0].getBuyPrice() * getSize();
//         }
//         return 0.0;
//     }

//     ~BondTransaction() override {}
// };

// class StockTransaction : public SecurityTransaction
// {
// private:
//     std::vector<Stock> stocks;
// public:
//     StockTransaction(std::string toEmail, Stock stock, int amount, TransactionType _type)
//         : SecurityTransaction(toEmail, _type) {
//             for (int i = 0; i < amount; i++) {
//                 stocks.push_back(stock);
//             }
//         }

//     size_t getSize() const
//     {
//         return stocks.size();
//     }

//     double getTransactionCost() const {
//         if getSize() > 0 {
//             return stocks[0].getBuyPrice() * getSize();
//         }
//         return 0.0;
//     }

//     ~StockTransaction() override {}
// };

template<typename T, typename U>
    requires Convertible<T, U>
class ConversionTransaction : public Transaction {
private:
    std::string toEmail;
    int amount;
public:
    ConversionTransaction(std::string toEmail, int amount) : Transaction(toEmail), toEmail(toEmail), amount(amount) {};
    void print() const override
    {
        std::cout << "Conversion amount: " << amount << std::endl;
    }
};

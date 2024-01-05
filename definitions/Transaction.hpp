#pragma once
#include "Event.hpp"
#include "Asset.hpp"
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

private:
    int amount;
};

template <typename T>
concept DerivedFromSecurity = std::is_base_of<Security, T>::value;

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

    double getTransactionCost() const
    {
        if (securities.size() > 0)
        {
            return securities[0].getBuyPrice() * securities.size();
        }
        return 0.0;
    }

    TransactionType getType() const
    {
        return type;
    }

    std::vector<T> &getSecurities()
    {
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

class ConversionTransaction : public Transaction
{
};

/*
template <typename T, typename U>
requires asset_is_convertible<T, U>
class ConversionTransaction : Transaction
{
private:
    T from;
    U to;
public:
    U convert(T from, U to);
};

*/

/*
    Concept til at gøre is_convertible_from_and_to et trait på alle transaktiontyper

    Stock og Bond er ikke konverterbare intern, men begge konverterbare til Cash
    Cash er konverterbar til begge typer.
*/

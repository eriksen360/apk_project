/**
 * @file Transaction.hpp
 *
 * @author Filip Møgelvang Hansen & Mathias Fenger-Eriksen
 *
 * @brief Transaction classes describing information about a transaction request.
 *
 */

#pragma once
#include "Event.hpp"
#include "Asset.hpp"
#include <tuple>
#include <set>
#include <concepts>
#include <type_traits>

/**
 * @brief Specifies which type of transaction is requested.
 *
 */
enum TransactionType
{
    BUY = 0,
    SELL = 1,
    CONVERT = 2
};

/**
 * @brief Base Transaction class.
 *
 */
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

/**
 * @brief Cash Transaction class used for cash transaction (Transfer, deposit, withdraw)
 *
 */
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

/**
 * @brief Security Transaction class used for security transactions (Stock, Bond)
 *
 * @tparam T Must be derived from Security. @see Asset.hpp.
 */
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

    double getTransactionBuyAmount() const
    {
        if (securities.size() > 0)
        {
            return securities[0].getBuyPrice() * securities.size();
        }
        return 0.0;
    }

    double getTransactionCurrentAmount() const
    {
        if (securities.size() > 0)
        {
            return securities[0].getCurrentPrice() * securities.size();
        }
        return 0.0;
    }

    double getTransactionSellAmount() const
    {
        if (securities.size() > 0)
        {
            return securities[0].getSellPrice() * securities.size();
        }
        return 0.0;
    }

    TransactionType getType() const
    {
        return type;
    }

    std::tuple<std::string, int> getSecuritiesOverview()
    {
        std::set<std::string> stockTypes;
        for (auto &security : securities)
        {
            stockTypes.insert(security.getName());
        }
        if (stockTypes.size() > 1)
        {
            throw std::logic_error("Stocks in transaction of different types.");
        }

        return std::tuple<std::string, int>(*stockTypes.begin(), securities.size());
    }

    std::vector<T> getSecurities() const
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

    void print() const override
    {
        std::cout << "Id: " << getId() << std::endl;
        std::cout << "Type: " << type << std::endl;
        std::cout << "Number: " << securities.size() << std::endl;
        std::cout << "Type: " << getType() << std::endl;
    }

    ~SecurityTransaction() {}
};

/**
 * @brief Conversion Transaction class using for converting assets. @see Asset.hpp for details on valid conversions.
 *
 * @tparam T Asset to convert from
 * @tparam U Asset to convert to
 */
template <typename T, typename U>
    requires Convertible<T, U>
class ConversionTransaction : public Transaction
{
private:
    std::string toEmail;
    int amount;

public:
    ConversionTransaction(std::string toEmail, int amount) : Transaction(toEmail), toEmail(toEmail), amount(amount){};
    void print() const override
    {
        std::cout << "Conversion amount: " << amount << std::endl;
    }
};

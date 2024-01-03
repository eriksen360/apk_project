#pragma once
#include <string>
#include <vector>
#include "Transaction.hpp"
#include <type_traits>
#include <concepts>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>

template <typename T>
class Account
{
public:
    Account(int id, std::string name, std::string userEmail) : id(id), name(std::move(name)), userEmail(userEmail) {}

    virtual void print() = 0;

    virtual void addToTransactionLog(T t)
    {
        transactions.push_back(t);
    }

    virtual int getID() const
    {
        return id;
    }

    std::string getName() const
    {
        return name;
    }

    std::string getUserEmail() const
    {
        return userEmail;
    }

private:
    int id;
    std::string name;
    std::string userEmail;
    std::vector<T> transactions;
};

class SavingsAccount : public Account<CashTransaction>
{
public:
    SavingsAccount(int id, std::string name, std::string userEmail, Cash cash)
        : Account<CashTransaction>(id, std::move(name), userEmail), cash(cash) {} // TODO: See exmaple in slides for std::moving own object

    int getAmount() const
    {
        return cash.getAmount();
    }

    void addAmount(int n)
    {
        cash.addAmount(n);
    }

    void removeAmount(int n)
    {
        cash.removeAmount(n);
    }

    Currency getCurrency() const
    {
        return cash.getCurrency();
    }

    void print() override
    {
        std::cout << "Type: Savings Account\n"
                  << "ID: " << this->getID() << '\n'
                  << "Name: " << this->getName() << '\n'
                  << "Amount: " << getAmount() << "\n\n";
    }

private:
    Cash cash; // overload operation med concept til at specificere at currency matcher cash.currency
};

template <typename T> // concept til at tjekke for stock eller bond
class SecuritiesAccount : public Account<T>
{
public:
    SecuritiesAccount(int id, std::string name, std::string userEmail)
        : Account<T>(id, std::move(name), userEmail) {}

    ~SecuritiesAccount(){};

    void print() override
    {
        std::cout << "Type: Securities Account\n"
                  << "ID: " << this->getID() << '\n'
                  << "Name: " << this->getName() << "\n\n";
    }

    void addSecurity(std::unique_ptr<Security> security)
    {
        securities.push_back(std::move(security));
    }

private:
    std::vector<std::unique_ptr<Security>> securities;
};

std::mutex databaseSavingsAccountsMutex;
std::mutex databaseStockAccountsMutex;
std::mutex databaseBondAccountsMutex;

struct DatabaseMock
{ // TODO: Use UIID keys
    std::unordered_map<int, SavingsAccount> savingAccounts;
    std::unordered_map<int, SecuritiesAccount<Stock>> stockAccounts;
    std::unordered_map<int, SecuritiesAccount<Bond>> bondAccounts;

    SavingsAccount *getSavingsAccountByEmail(const std::string &email)
    {
        for (auto &pair : savingAccounts)
        {
            SavingsAccount &account = pair.second;
            if (account.getUserEmail() == email)
            {
                return &account; // Return the address of the account
            }
        }
        return nullptr; // Return nullptr if not found
    }

    bool accountExistsFor(std::string userEmail)
    {

        // Use iterator + algorithm to find account in any of the maps
    }

    void displayAccountsFor(std::string userEmail)
    {

        // Use iterator + algorithm to find account in any of the maps
        // Call .print() on each account
    }
};

// // You should not be able to make a strockTransaction if you
// // have no stockAccount

// // Concept to specify that only stocktransactions and
// // stockassets are allowed

// template <typename X>
// concept SecurityTransaction = std::is_same<X, StockTransaction> || std::is_same<X, BondTransaction> || std::is_same<X, ConversionTransaction>;

// template <typename X>
// concept SecurityAsset = std::is_same<X, Stock> || std::is_same<X, Bond>;

// template <typename X>
// concept _CashTransaction = std::same_as<X, CashTransaction>;

// // trait til cash

// // Concept til at definere U som Stock eller Bondtransaction
// template<typename M, typename U>
// requires SecurityTransaction<U>
// requires SecurityAsset<M>
// class SecuritiesAccount : Account<U> {
// private:
//     std::vector<M> securities;
// public:
//     virtual void addToTransactionLog(T transaction);
// };

// // TODO: Rule of 5!!! Do it everywhere. Move constructors only make sense
// // if the resource is not unique. You cannot move a

// class A {

// };

// enum Currency {
//     DKK = 0,
//     USD = 1
// };

// // Concept til at definere trait for T std::is_of_type_int<T>
// template<typename T>
// requires std::is_integral<T>
// class SavingsAccount : Account<CashTransaction, ConversionTransaction> { // <CashTransaction> her?

// // Type kan kun være cash
// private:
//     int amount;
//     Currency currency;  // get only property
//     SavingsAccount(Currency currency, T amount = 0);

//     // Must be cash and come from sale of stock or bond
//     // or regular deposit or withdrawel
//     type_of(transactions[0]::fromTransactionType)

// };

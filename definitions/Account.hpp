#include <string>
#include <vector>
#include "Transaction.hpp"
#include "Asset.hpp"
#include <type_traits>
#include <concepts>
#include <iostream>

enum Currency
{
    DKK = 0,
    USD = 1
};

template <typename T>
class Account
{
public:
    Account(int id, std::string name) : id(id), name(std::move(name)) {}

    virtual void print() = 0;

    virtual void addToTransactionLog(T t){
        transactions.push_back(t);
    }

    virtual int getID() const{
        return id;
    }

    virtual std::string getName() const {
        return name;
    }

private:
    int id;
    std::string name;
    std::vector<T> transactions;
};

class SavingsAccount : public Account<CashTransaction>
{
public:
    SavingsAccount(int id, std::string name, Currency currency, int amount = 0)
        : Account<CashTransaction>(id, std::move(name)), currency(currency), amount(amount) {}

    int getAmount() const {
        return amount;
    }

    void addAmount(int n){
        amount += n;
    }

    void removeAmount(int n){
        amount -= n;
    }

    void print() override {
        std::cout << "Type: Savingsaccount\n"
                  << "ID: " << getID() << '\n'
                  << "Name: " << getName() << '\n'
                  << "Amount: " << getAmount() << "\n\n";
    }

private:
    int amount;
    Currency currency;
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
// concept _CashTransaction = std::is_same<X, CashTransaction>;

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

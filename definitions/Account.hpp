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

namespace bank {

    template<typename T>
    class Account
    {
    public:
        Account(int id, std::string name, std::string userEmail) : id(id), name(std::move(name)), userEmail(userEmail) {}

        virtual void print() = 0;

        virtual void addToTransactionLog(T& t)
        {
            transactions.push_back(std::move(t));
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
        const int id;
        const std::string name;
        const std::string userEmail;
        std::vector<T> transactions;
    };

    class SavingsAccount : public Account<CashTransaction>
    {
    public:
        SavingsAccount(int id, std::string name, std::string userEmail, Cash&& cash)
            : Account<CashTransaction>(id, std::move(name), userEmail), cash(std::move(cash)) {} // TODO: See exmaple in slides for std::moving own object

        int getAmount() const
        {
            return cash.getAmount();
        }

        void addAmount(int n) // noexcept
        {
            cash.addAmount(n);
        }

        void reduceAmount(int n) // noexcept
        {
            cash.reduceAmount(n);
        }

        Currency getCurrency() const
        {
            return cash.getCurrency();
        }

        void print() override
        {
            std::cout << "Type: Savings Account\n"
                    << "ID: " << this->getID() << '\n'
                    << "Email: " << this->getUserEmail() << '\n'
                    << "Name: " << this->getName() << '\n'
                    << "Amount: " << getAmount() << "\n\n";
        }

    private:
        Cash cash; // overload operation med concept til at specificere at currency matcher cash.currency
    };

    template <typename A, typename T> // concept til at tjekke for stock eller bond, og concept til at tjekke for at T er en bestemt tranaktionstype
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
                    << "Name: " << this->getName() << "\n\n"
                    << "Email: " << this->getUserEmail() << '\n'
                    << "Total asset value: " << getTotalAssetValue() << '\n'
                    << "Mean asset value: " << getMeanAssetValue() << '\n';
        }

        void addSecurity(A& security)
        {
            securities.push_back(std::move(security));
        }

        double getTotalAssetValue() {
            // Implement iterator for summation
            return 0.0;
        }

        double getMeanAssetValue() const {
            // Implement algorithm for calculating
            return 0.0;
        }

        void addSecurities(std::vector<A> _securities) {
            // TODO: Implement move properly
            securities.insert( securities.end(), _securities.begin(), _securities.end() );
        }

        std::vector<A> returnSecurities(T security, int amount) {
            std::vector<A> tmp;
            return tmp;
            // Iterate over securities

                // If we cannot find amount security -> Discard operaiton adn throw exception
                // Else add to tmp vector and return
        }

    private:
        std::vector<A> securities;
        // std::vector<unique_ptr<A>> securities;  <--- Det her er den ikke glad for, fordi typen A ikke er deducable 
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

}

// namespace database {
std::mutex databaseSavingsAccountsMutex;
std::mutex databaseStockAccountsMutex;
std::mutex databaseBondAccountsMutex;

struct DatabaseMock { 
    /*
        We have chosen to allow only one account type per user. This makes the use of
        an unordered_map ideal as we can index the account in O(1), even when there
        are a significant amount of accounts.
    */
    std::unordered_map<std::string, bank::SavingsAccount> savingsAccounts;
    std::unordered_map<std::string, bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>>> stockAccounts;
    std::unordered_map<std::string, bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>>> bondAccounts;

    bool accountExistsFor(std::string userEmail)
    {
        if (this->savingsAccounts.find(userEmail) != this->savingsAccounts.end()) {
            return true;
        }
        if (this->stockAccounts.find(userEmail) != this->stockAccounts.end()) {
            return true;
        }
        if (this->bondAccounts.find(userEmail) != this->bondAccounts.end()) {
            return true;
        }
        return false;
    }

    void displayAccountsFor(std::string userEmail)
    {
        auto savingsElement = this->savingsAccounts.find(userEmail);
        if (savingsElement != this->savingsAccounts.end()) {
            savingsElement->second.print();
        }

        auto stockElement = this->stockAccounts.find(userEmail);
        if (stockElement != this->stockAccounts.end()) {
            stockElement->second.print();
        }

        auto bondElement = this->bondAccounts.find(userEmail);
        if (bondElement != this->bondAccounts.end()) {
            bondElement->second.print();
        }
    }

    bank::SavingsAccount* getSavingsAccountFor(std::string userEmail) {
        auto element = this->savingsAccounts.find(userEmail);
        if (element != this->savingsAccounts.end()) {
            return &(element->second);
        }
        return nullptr;
    }

    bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>>* getStockAccountFor(std::string userEmail) {
        auto element = this->stockAccounts.find(userEmail);
        if (element != this->stockAccounts.end()) {
            return &(element->second);
        }
        return nullptr;
    }

    bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>>* getBondAccountFor(std::string userEmail) {
        auto element = this->bondAccounts.find(userEmail);
        if (element != this->bondAccounts.end()) {
            return &(element->second);
        }
        return nullptr;
    }
};
//}
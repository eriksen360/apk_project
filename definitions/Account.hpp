/**
 * @file Account.hpp
 *
 * @author Filip Møgelvang Hansen & Mathias Fenger-Eriksen
 *
 * @brief Account classes, wrapped in a bank namespace.
 *
 */

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
#include <algorithm>

namespace bank
{
    /**
     * @brief Base account class.
     *
     * @tparam T Type of transactions
     */
    template <typename T>
    class Account
    {
    public:
        Account(std::string name, std::string userEmail) : name(std::move(name)), userEmail(userEmail)
        {
            boost::uuids::random_generator gen;
            boost::uuids::uuid _id = gen();
            id = _id;
        }

        virtual void print() = 0;

        virtual void addToTransactionLog(T &&t)
        {
            transactions.push_back(std::move(t));
        }

        boost::uuids::uuid getID() const
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

        std::vector<T> getTransactions() const
        {
            return transactions;
        }

        void printTransactions() const
        {
            for (auto &transaction : this->getTransactions())
            {
                transaction.print();
            }
        }

    private:
        std::vector<T> transactions;
        boost::uuids::uuid id;
        std::string name;
        std::string userEmail;
    };

    /**
     * @brief Savings account used for Cash.
     *
     */
    class SavingsAccount : public Account<CashTransaction>
    {
    public:
        SavingsAccount(std::string name, std::string userEmail, Cash &&cash)
            : Account<CashTransaction>(std::move(name), std::move(userEmail)), cash(std::move(cash)) {}

        int getAmount() const
        {
            return cash.getAmount();
        }

        void addAmount(int n) noexcept
        {
            cash.addAmount(n);
        }

        void reduceAmount(int n) noexcept
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
                      << "Amount: " << getAmount() << "\n"
                      << "-- TRANSACTIONS -- \n";
            this->printTransactions();
        }

    private:
        Cash cash;
    };

    /**
     * @brief Security account used for either Stock or Bond
     *
     * @tparam A Type derived from Security (Stock or Bond)
     * @tparam T Type of transaction
     */
    template <typename A, typename T>
        requires DerivedFromSecurity<A>
    class SecuritiesAccount : public Account<T>
    {
    public:
        SecuritiesAccount(std::string name, std::string userEmail)
            : Account<T>(std::move(name), std::move(userEmail)) {}

        ~SecuritiesAccount(){};

        void print() override
        {
            std::cout << "Type: Securities Account\n"
                      << "ID: " << this->getID() << '\n'
                      << "Name: " << this->getName() << "\n"
                      << "Email: " << this->getUserEmail() << '\n'
                      << "Total asset value: " << getTotalAssetValue() << '\n'
                      << "Mean asset value: " << getMeanAssetValue() << '\n'
                      << "-- TRANSACTIONS -- \n";
            this->printTransactions();
        }

        void addSecurity(A &&security)
        {
            securities.push_back(std::move(security));
        }

        double getTotalAssetValue()
        {
            if (securities.empty())
            {
                return 0;
            }
            double assetValueSum;
            for (unsigned int i = 0; i < securities.size(); i++)
            {
                assetValueSum += securities[i].getCurrentPrice();
            }
            return assetValueSum;
        }

        double getMeanAssetValue()
        {
            if (securities.empty())
            {
                return 0;
            }
            return (getTotalAssetValue() / securities.size());
        }

        void addSecurities(std::vector<A> &&_securities)
        {
            std::move(_securities.begin(), _securities.end(), std::back_inserter(securities));
        }

        bool hasSpecificSecurities(std::string securityName, int amount)
        {
            size_t n_securities = 0;
            for (auto &accountSecurity : securities)
            {
                if (accountSecurity.getName() == securityName)
                {
                    n_securities++;
                }
            }

            if (n_securities < amount)
            {
                return false;
            }
            return true;
        }

        void removeSecurities(std::string securityName, int amount)
        {
            std::vector<int> indiciesToRemove;
            for (int i = 0; i < securities.size(); i++)
            {
                if (securities[i].getName() == securityName)
                {
                    indiciesToRemove.push_back(i);
                }
            }
            if (indiciesToRemove.size() < amount)
            {
                throw std::logic_error("Cannot remove from account as not enough stocks.");
            }
            for (int i = 0; i < amount; i++)
            {
                securities.erase(securities.begin() + indiciesToRemove[i] - i);
            }
        }

    private:
        std::vector<A> securities;
    };

}

std::mutex databaseSavingsAccountsMutex;
std::mutex databaseStockAccountsMutex;
std::mutex databaseBondAccountsMutex;

/**
 * @brief Database for storing accounts.
 *
 */
struct DatabaseMock
{
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
        if (this->savingsAccounts.find(userEmail) != this->savingsAccounts.end())
        {
            return true;
        }
        if (this->stockAccounts.find(userEmail) != this->stockAccounts.end())
        {
            return true;
        }
        if (this->bondAccounts.find(userEmail) != this->bondAccounts.end())
        {
            return true;
        }
        return false;
    }

    void displayAccountsFor(std::string userEmail)
    {
        getSavingsAccountFor(userEmail)->print();
        getStockAccountFor(userEmail)->print();
        getBondAccountFor(userEmail)->print();
    }

    bank::SavingsAccount *getSavingsAccountFor(std::string userEmail)
    {
        auto element = this->savingsAccounts.find(userEmail);
        if (element != this->savingsAccounts.end())
        {
            return &(element->second);
        }
        return nullptr;
    }

    bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> *getStockAccountFor(std::string userEmail)
    {
        auto element = this->stockAccounts.find(userEmail);
        if (element != this->stockAccounts.end())
        {
            return &(element->second);
        }
        return nullptr;
    }

    bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>> *getBondAccountFor(std::string userEmail)
    {
        auto element = this->bondAccounts.find(userEmail);
        if (element != this->bondAccounts.end())
        {
            return &(element->second);
        }
        return nullptr;
    }
};
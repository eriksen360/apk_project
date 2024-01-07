/**
 * @file main.cpp
 *
 * @brief SWAPK Project. 2024-01-08
 *        A simple simulator for banking transactions. Based on
 *        an eventqueue, which stores all the requests from the client.
 *        Transactions process takes care of the transactions, based on the asset type.
 *        The asset type can be Cash, Stock or Bond.
 *
 * @author Filip Møgelvang Hansen & Mathias Fenger-Eriksen
 */

#include <iostream>
#include <vector>
#include <typeinfo>
#include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
#include "definitions/Event.hpp"
#include <tuple>
#include <memory>
#include <chrono>
#include <thread>

/**
 * Globals
 */
DatabaseMock database;
bank::EventMessageQueue<std::unique_ptr<Event>> event_queue; // Main event queue. Only supports Event-derived types.
                                                             // unique_ptr is used as it is not
                                                             // possible to create intances of Event
                                                             // because it is abstract

/**
 * @brief Cash transaction task. Takes care of Cash transactions.
 *
 */
void cashTransactionThreadFunction()
{
    for (;;)
    {
        if (event_queue.frontIsOfType(typeid(CashTransaction))) // ADL kicks in because event_queue is in 'bank::' namespace
        {
            std::cout << "CASH TYPE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue();               // Retrieve the event from the queue
            auto cashTransaction = static_cast<CashTransaction *>(event.get()); // Cast to Cashtransaction because event_queue
                                                                                // is of type Event, which i a base class.

            int amount = cashTransaction->getAmount(); // Get attributes
            std::string toEmail = cashTransaction->getToAccount();
            std::string fromEmail = cashTransaction->getFromAccount();
            std::cout << "Transfer " << amount << " to " << toEmail << " from " << fromEmail << std::endl;

            std::scoped_lock<std::mutex> lock(databaseSavingsAccountsMutex);          // (scoped)Lock mutex when accessing the database
            bank::SavingsAccount *toAccount = database.getSavingsAccountFor(toEmail); // Get accounts
            bank::SavingsAccount *fromAccount = database.getSavingsAccountFor(fromEmail);
            if (toAccount != nullptr)
            {
                if (fromAccount != nullptr)
                {
                    if (fromAccount->getAmount() < amount)
                    {
                        
                        try {
                            bank::SavingsAccount fromAccountTmp = *fromAccount;
                            fromAccountTmp.addToTransactionLog(std::move(*cashTransaction));

                            bank::SavingsAccount toAccountTmp = *toAccount;
                            toAccountTmp.addToTransactionLog(std::move(*cashTransaction));

                            // No-throw guaranteed operations
                            fromAccount = &fromAccountTmp;
                            toAccount = &toAccountTmp;
                            fromAccount->reduceAmount(amount);
                            toAccount->addAmount(amount);
                        } catch (std::exception& e) {
                            std::cout << e.what() << std::endl;
                            // revert accounts to previous state
                            continue;
                        }
                        std::cout << amount << " transfered to account " << toAccount << " with success." << std::endl;
                    }
                    else {
                        std::cout << "Not enough funds to transfer amount " << amount << " from account " << fromAccount << std::endl; 
                    }
                }
                else
                {
                    try {
                        bank::SavingsAccount toAccountTmp = *toAccount;
                        toAccountTmp.addToTransactionLog(std::move(*cashTransaction));
                
                        // No-throw guaranteed operations
                        toAccount = &toAccountTmp;
                        toAccount->addAmount(amount);
                    
                    } catch (std::exception& e) {
                        std::cout << e.what() << std::endl;
                        // revert accounts to previous state
                        continue;
                    }
                    std::cout << amount << " Deposited to account " << toAccount << " with success." << std::endl;
                }
            }
            else
            {
                std::cout << "No to_account -> Transaction Error." << std::endl;
            }
        }
    }
}

/**
 * @brief Security transaction task. Takes care of Stock and Bond transactions.
 *
 */
void SecurityTransactionThreadFunction()
{
    for (;;)
    {
        if (event_queue.frontIsOfType(typeid(SecurityTransaction<Stock>))) // Check for Stock transaction
        {
            std::cout << "Stock Type in Queue: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue();
            SecurityTransaction<Stock> *transaction = static_cast<SecurityTransaction<Stock> *>(event.get());

            if (!transaction->is_stock())
            { // Use constexpr to double check
                std::cout << "Transaction is not a stock transaction. Skipping." << std::endl;
                continue;
            }

            std::scoped_lock<std::mutex> savingsAccountLock(databaseSavingsAccountsMutex);
            bank::SavingsAccount *savingsAccount = database.getSavingsAccountFor(transaction->getToAccount());
            if (savingsAccount == nullptr)
            {
                std::cout << "No savings account found for user. Cannot withdraw/deposit funds made by selling/buying stocks." << std::endl;
                continue;
            }

            if (transaction->getType() == TransactionType::BUY)
            {

                std::tuple<std::string, int> securityType;
                try {
                    securityType = transaction->getSecuritiesOverview();
                } catch (std::logic_error& e) {
                    std::cout << e.what() << std::endl;
                    continue;
                }

                if (transaction->getTransactionBuyAmount() > savingsAccount->getAmount())
                {
                    std::cout << "Not enough funds to buy stocks." << std::endl;
                    continue;
                }

                std::scoped_lock<std::mutex> stockAccountLock(databaseStockAccountsMutex);
                bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> *securityAccount = database.getStockAccountFor(transaction->getToAccount());
                if (securityAccount == nullptr)
                {
                    std::cout << "No bond account found for user. Cannot transfer bonds." << std::endl;
                    continue; // lock released by exiting scope of loop.
                }

                try {
                    SecurityTransaction<Stock> transactionTmp = *transaction;
                    std::vector<Stock> securities = transactionTmp.getSecurities();

                    double transactionCost = transactionTmp.getTransactionBuyAmount();
                    bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> securityAccountTmp = *securityAccount;
                    securityAccountTmp.addToTransactionLog(std::move(transactionTmp));
                    securityAccountTmp.addSecurities(std::move(securities));
                    
                    // No-throw guaranteed operations
                    securityAccount = &securityAccountTmp; 
                    savingsAccount->reduceAmount(transactionCost);
                } catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                    // revert accounts to previous state
                    continue;
                }

            }
            else if (transaction->getType() == TransactionType::SELL)
            {
                std::tuple<std::string, int> securityType;
                try {
                    securityType = transaction->getSecuritiesOverview();
                } catch (std::logic_error& e) {
                    std::cout << e.what() << std::endl;
                    continue;
                }
                
                std::scoped_lock<std::mutex> securityAccountLock(databaseStockAccountsMutex);
                bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> *securityAccount = database.getStockAccountFor(transaction->getToAccount());
                if (securityAccount == nullptr)
                {
                    std::cout << "No bond account found for user. Cannot transfer bonds." << std::endl;
                    continue; // lock released by exiting scope of loop.
                }

                try {
                    if (!securityAccount->hasSpecificSecurities(std::get<0>(securityType), std::get<1>(securityType)))
                    {
                        std::cout << "Not enough securities to sell." << std::endl;
                        continue;
                    }
                } catch (std::logic_error& e) {
                    std::cout << e.what() << std::endl;
                    continue;
                }
                
                try {
                    SecurityTransaction<Stock> transactionTmp = *transaction;
                    std::vector<Stock> securities = transactionTmp.getSecurities();

                    bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> securityAccountTmp = *securityAccount;
                    securityAccount->removeSecurities(std::get<0>(securityType), std::get<1>(securityType));     

                    // No-throw guaranteed operations
                    securityAccount = &securityAccountTmp; 
                    savingsAccount->addAmount(transaction->getTransactionSellAmount()); 
                } catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                    // revert accounts to previous state
                    continue;
                }
            }
        }

        else if (event_queue.frontIsOfType(typeid(SecurityTransaction<Bond>))) // Check for Bond transaction
        {
            std::cout << "Bond Type in Queue: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue();
            SecurityTransaction<Bond> *transaction = static_cast<SecurityTransaction<Bond> *>(event.get());

            if (!transaction->is_bond())
            { // Use constexpr to double check
                std::cout << "Transaction is not a bond transaction. Skipping." << std::endl;
                continue;
            }

            std::scoped_lock<std::mutex> savingsAccountLock(databaseSavingsAccountsMutex);
            bank::SavingsAccount *savingsAccount = database.getSavingsAccountFor(transaction->getToAccount());
            if (savingsAccount == nullptr)
            {
                std::cout << "No savings account found for user. Cannot withdraw/deposit funds made by selling/buying bonds." << std::endl;
                continue;
            }

            if (transaction->getType() == TransactionType::BUY)
            {

                std::tuple<std::string, int> securityType;
                try {
                    securityType = transaction->getSecuritiesOverview();
                } catch (std::logic_error& e) {
                    std::cout << e.what() << std::endl;
                    continue;
                }

                if (transaction->getTransactionBuyAmount() > savingsAccount->getAmount())
                {
                    std::cout << "Not enough funds to buy bonds." << std::endl;
                    continue;
                }

                std::scoped_lock<std::mutex> securityAccountLock(databaseBondAccountsMutex);
                bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>> *securityAccount = database.getBondAccountFor(transaction->getToAccount());
                if (securityAccount == nullptr)
                {
                    std::cout << "No bond account found for user. Cannot transfer bonds." << std::endl;
                    continue; // lock released by exiting scope of loop.
                }

                try {
                    SecurityTransaction<Bond> transactionTmp = *transaction;
                    std::vector<Bond> securities = transactionTmp.getSecurities();

                    bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>> securityAccountTmp = *securityAccount;
                    securityAccountTmp.addSecurities(std::move(securities));
                    double transactionCost = transactionTmp.getTransactionBuyAmount();
                    securityAccountTmp.addToTransactionLog(std::move(transactionTmp));

                    // No-throw guaranteed operations
                    securityAccount = &securityAccountTmp; 
                    savingsAccount->reduceAmount(transactionCost);
                } catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                    // revert accounts to previous state
                    continue;
                }

            }
            else if (transaction->getType() == TransactionType::SELL)
            {
                std::tuple<std::string, int> securityType;
                try {
                    securityType = transaction->getSecuritiesOverview();
                } catch (std::logic_error& e) {
                    std::cout << e.what() << std::endl;
                    continue;
                }
                
                std::scoped_lock<std::mutex> bondAccountLock(databaseBondAccountsMutex);
                bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>> *securityAccount = database.getBondAccountFor(transaction->getToAccount());
                if (securityAccount == nullptr)
                {
                    std::cout << "No bond account found for user. Cannot transfer bonds." << std::endl;
                    continue; // lock released by exiting scope of loop.
                }

                try {
                    if (!securityAccount->hasSpecificSecurities(std::get<0>(securityType), std::get<1>(securityType)))
                    {
                        std::cout << "Not enough securities to sell." << std::endl;
                        continue;
                    }
                } catch (std::logic_error& e) {
                    std::cout << e.what() << std::endl;
                    continue;
                }
                
                try {
                    SecurityTransaction<Bond> transactionTmp = *transaction;
                    std::vector<Bond> securities = transactionTmp.getSecurities();

                    bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>> securityAccountTmp = *securityAccount;
                    securityAccount->removeSecurities(std::get<0>(securityType), std::get<1>(securityType));     
                    securityAccountTmp.addToTransactionLog(std::move(transactionTmp));

                    // No-throw guaranteed operations
                    securityAccount = &securityAccountTmp; 
                    savingsAccount->addAmount(transaction->getTransactionSellAmount()); 
                } catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                    // revert accounts to previous state
                    continue;
                }
            }
        }
    }
}

/**
 * @brief Conversion transaction task. Takes care of Conversion transactions.
 *
 */
void ConversionTransactionThreadFunction()
{

    for (;;)
    {
        // Limitation of typeid. Cannot check typeid of template class without template arguments.
        if (event_queue.frontIsOfType(typeid(ConversionTransaction<Cash, Stock>))) {
            std::cout << "Conversion Type in Queue: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue();
            ConversionTransaction<Cash, Stock> *transaction = static_cast<ConversionTransaction<Cash, Stock> *>(event.get());
            
            // Handle Conversion
        }
         
    }
}

/**
 * @brief Main program loop. Acts as Client and sends transaction requests to the queue.
 *
 */
int main()
{
    std::thread cashTransactionThread(cashTransactionThreadFunction); // Create threads
    std::thread securitiesTransactionThread(SecurityTransactionThreadFunction);
    std::thread conversionTransactionThread(ConversionTransactionThreadFunction);

    cashTransactionThread.detach(); // Detach threads
    securitiesTransactionThread.detach();
    conversionTransactionThread.detach();

    bank::SavingsAccount s1("Opsparingskonto", "a.jensen@gmail.com", std::move(Cash(10000, DKK))); // Create test accounts and add to database
    bank::SavingsAccount s2("Opsparingskonto", "b.hansen@gmail.com", std::move(Cash(5400, DKK)));
    database.savingsAccounts.emplace(s1.getUserEmail(), std::move(s1));
    database.savingsAccounts.emplace(s2.getUserEmail(), std::move(s2));

    bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> s3("Aktiekonto", "a.jensen@gmail.com");
    bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> s4("Aktiekonto", "b.hansen@gmail.com");
    database.stockAccounts.emplace(s3.getUserEmail(), std::move(s3));
    database.stockAccounts.emplace(s4.getUserEmail(), std::move(s4));

    bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>> s5("Obligationskonto", "a.jensen@gmail.com");
    bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>> s6("Obligationskonto", "b.hansen@gmail.com");
    database.bondAccounts.emplace(s5.getUserEmail(), std::move(s5));
    database.bondAccounts.emplace(s6.getUserEmail(), std::move(s6));

    std::cout << "Savings Accounts ------------------------" << std::endl;
    for (auto &x : database.savingsAccounts)
    {
        x.second.print();
    }

    std::cout << "Stock Accounts --------------------------" << std::endl;
    for (auto &x : database.stockAccounts)
    {
        x.second.print();
    }
    std::cout << "Bond Accounts ---------------------------" << std::endl;
    for (auto &x : database.bondAccounts)
    {
        x.second.print();
    }
    std::cout << "------------------------------------------" << std::endl;

    std::unique_ptr<Event> eventRequest; // Send condition on eventRequest
    std::string userEmail;
    do
    {
        std::cout << "User logged in as: ";
        std::cin >> userEmail;
        if (!database.accountExistsFor(userEmail))
        {
            std::cout << "User does not exist. Please try again." << std::endl;
        }
        else
        {
            database.displayAccountsFor(userEmail);
            break;
        }
    } while (1);

    char choice = 0;
    do
    {
        std::cout << "Please enter action 1 (Make deposit), 2 (Transfer), 3 (Trade Stocks), 4 (Trade Bonds), 5 (Convert Assets), 6 (Show accounts)" << std::endl;
        try
        {
            std::cin.ignore();
            std::cin >> choice;
            if (choice < 48 || choice > 57) // C = {0..9}  TODO: Should be on illegal type
            {
                throw std::invalid_argument(&choice);
            }
        }
        catch (std::invalid_argument const &ex)
        {
            std::cout << "Please enter a valid choice [Integer range 0-255], and not" << ex.what() << std::endl;
            choice = '0';
        }

        int amount = 0;
        switch (choice) // Assumes that Client (this) has no access to Accounts for now, so any mistake will be handled silently
        // in the respective thread. Client should return a Future to a list of requests, that the customer can then access
        {
            case '1': // Make deposit
            {
                std::cout << "Please enter amount to deposit: ";
                std::cin >> amount;
                if (amount <= 0)
                {
                    std::cout << "Amount must be positive." << std::endl;
                    break;
                } 

                eventRequest = std::make_unique<CashTransaction>(amount, userEmail); // wrap event object in unique_ptr
                event_queue.enqueue(std::move(eventRequest));                        // move the unique_ptr of the event to the queue
                break;
            }
            case '2': // Transfer
            {
                std::cout << "Please enter amount to transfer: ";
                std::cin >> amount;
                if (amount <= 0)
                {
                    std::cout << "Amount must be positive." << std::endl;
                    break;
                } 

                std::string toEmail;
                std::cout << "Please enter email to transfer to: ";
                std::cin >> toEmail;
                eventRequest = std::make_unique<CashTransaction>(amount, toEmail, userEmail); // wrap event object in unique_ptr
                event_queue.enqueue(std::move(eventRequest));                                 // move the unique_ptr of the event to the queue
                break;
            }
            case '3': // Buy|Sell Stocks
            {
                std::string stockName;
                for (auto &stock : availableStocks)
                {
                    stock.second.print();
                }
                do
                {
                    std::cout << "\nPlease enter stock name: ";
                    std::cin >> stockName;
                } while (!availableStocks.contains(stockName));
                std::cout << "Please enter amount to buy: ";
                std::cin >> amount;
                if (amount <= 0)
                {
                    std::cout << "Amount must be positive." << std::endl;
                    break;
                } 

                std::string transactionTypeStr;
                TransactionType transactionType;
                do
                {
                    std::cout << "\nPlease enter what to do with stock (SELL/BUY): ";
                    std::cin >> transactionTypeStr;
                    if (transactionTypeStr == "BUY")
                    {
                        transactionType = TransactionType::BUY;
                    }
                    else if (transactionTypeStr == "SELL")
                    {
                        transactionType = TransactionType::SELL;
                    }
                } while (transactionType != TransactionType::SELL && transactionType != TransactionType::BUY);
                std::cout << transactionTypeStr << amount << " " << stockName << " stocks." << std::endl;

                auto stockElem = availableStocks.find(stockName);
                if (stockElem == availableStocks.end())
                {
                    std::cout << "Stock not found." << std::endl;
                    break;
                }
                Stock stock = stockElem->second;
                std::cout << "id: " << stock.getID() << std::endl;
                eventRequest = std::make_unique<SecurityTransaction<Stock>>(userEmail, std::move(stock), amount, transactionType); // wrap event object in unique_ptr
                event_queue.enqueue(std::move(eventRequest));                                                                      // move the unique_ptr of the event to the queue
                break;
            }
            case '4': // Buy|Sell Bonds
            {
                std::string bondName;
                for (auto &bond : availableBonds)
                {
                    bond.second.print();
                }
                do
                {
                    std::cout << "\nPlease enter bond name: ";
                    std::cin >> bondName;
                } while (!availableBonds.contains(bondName));
                std::cout << "Please enter amount to buy: ";
                std::cin >> amount;
                if (amount <= 0)
                {
                    std::cout << "Amount must be positive." << std::endl;
                    break;
                } 

                std::string transactionTypeStr;
                TransactionType transactionType;
                do
                {
                    std::cout << "\nPlease enter what to do with bond (SELL/BUY): ";
                    std::cin >> transactionTypeStr;
                    if (transactionTypeStr == "BUY")
                    {
                        transactionType = TransactionType::BUY;
                    }
                    else if (transactionTypeStr == "SELL")
                    {
                        transactionType = TransactionType::SELL;
                    }
                } while (transactionType != TransactionType::SELL && transactionType != TransactionType::BUY);
                std::cout << transactionTypeStr << " " << amount << " " << bondName << " bonds." << std::endl;

                auto bondElem = availableBonds.find(bondName);
                if (bondElem == availableBonds.end())
                {
                    std::cout << "Bond not found." << std::endl;
                    break;
                }
                Bond bond = bondElem->second;
                std::cout << "id: " << bond.getID() << std::endl;
                eventRequest = std::make_unique<SecurityTransaction<Bond>>(userEmail, std::move(bond), amount, transactionType); // wrap event object in unique_ptr
                event_queue.enqueue(std::move(eventRequest));                                                                      // move the unique_ptr of the event to the queue
                break;
            }
            case '5':
            {
                char answer = 'N';
                do {
                    std::cout << "Perform conversion? (Y/N): ";
                    std::cin >> answer;
                } while(answer != 'Y');
                eventRequest = std::make_unique<ConversionTransaction<Cash, Stock>>(userEmail, amount);
                event_queue.enqueue(std::move(eventRequest));
                break;
            }
            case '6':
            {
                database.displayAccountsFor(userEmail);
                break;
            }
            default:
                break;
        }
    } while (choice != 'c');

    // Cleanup and exit
    // Not possible to call CashTransaction specific function without
    // casting the Event ptr to CashTransaction ptr.
    // Should be used with caution, as it is done runtime
    // Skal laves om til exception hvis den fejler
    // if (auto cashTransaction = static_cast<CashTransaction *>(eventHandler.get()))
    // {
    //     s1.reduceAmount(cashTransaction->getAmount());
    //     s2.addAmount(cashTransaction->getAmount());
    // }

    for (auto &x : database.savingsAccounts)
    {
        x.second.print();
    }

    return 0;
}

/*
    Publisher-Consumer Pattern

    3 threads consuming from a queue
        StockTransactionThread
        SavingsTransactionThread
        CompanyTransactionThread

    // Each thread checks the message to see if it should handle the type of event.
    // Events can be any change to account state. Use futures to get data with std::move?

    // We should use templates for all data
    // Custom exceptions for handling bad flow paths
*/

#include <iostream>
#include <vector>
#include <typeinfo>
// #include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
// #include "definitions/Event.hpp"
#include <memory>

DatabaseMock database;
bank::EventMessageQueue<std::unique_ptr<Event>> event_queue; // Main event queue. Only supports Event-derived types.
                                                       // unique_ptr is used as it is not
                                                       // possible to create intances of Event
                                                       // because it is abstract

void cashTransactionThreadFunction()
{
    for (;;)
    {
        if (event_queue.frontIsOfType(typeid(CashTransaction))) // ADL kicks in because event_queue is in 'bank::' namespace
        {
            std::cout << "CASH TYPE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue(); // get the event
            auto cashTransaction = static_cast<CashTransaction *>(event.get()); // cast to Cashtransaction
            // auto cashTransactionPtr = std::unique_ptr<CashTransaction>(*cashTransaction); // wrap in unique_ptr

            int amount = cashTransaction->getAmount();
            std::string toEmail = cashTransaction->getToAccount();
            std::string fromEmail = cashTransaction->getFromAccount();

            std::cout << "Transfer " << amount << " to " << toEmail << " from " << fromEmail << std::endl;

            std::scoped_lock<std::mutex> lock(databaseSavingsAccountsMutex);
            bank::SavingsAccount* toAccount = database.getSavingsAccountFor(toEmail);
            bank::SavingsAccount* fromAccount = database.getSavingsAccountFor(fromEmail);
            if (toAccount != nullptr)
            {
                if (fromAccount != nullptr)
                {
                    // TODO: This is a nothrow guarantee?
                    fromAccount->reduceAmount(amount);
                    toAccount->addAmount(amount);
                    std::cout << amount << " transfered to account " << toAccount << " with success." << std::endl;

                    // TODO: Discard if transaction is not logged properly? Strong guarantee?
                    // toAccount->addToTransactionLog(std::move(cashTransaction))
                    toAccount->addToTransactionLog(*cashTransaction);
                }
                else
                {
                    toAccount->addAmount(amount);
                    std::cout << amount << " transfered to account " << toAccount << " with success." << std::endl;
                    // TODO: Discard if transaction is not logged properly? Strong guarantee?
                    //toAccount->addToTransactionLog(std::move(cashTransaction))
                    toAccount->addToTransactionLog(*cashTransaction);
                    std::cout << "No from_account -> This is a deposit." << std::endl;
                }
            }
            else
            {
                std::cout << "No to_account -> Transaction Error." << std::endl;
            }
            // Scoped_lock releases mutex? How does mutex work in namespace compared to global?

            // -----------------------------------
            // Handle Transaction depending on if deposit, withdrawel or transfer
            // if (Transaction.from) {
            //     bank::SavingsAccount fromSavingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
            //     fromSavingsAccount.reduceAmout(transaction->getAmount());
            // }
            // Strong guarantee that amount must be added only if removed
            // bank::SavingsAccount toSavingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
            // toSavingsAccount.addAmount(transaction->getAmount());
            // Save transaction to appropiate account
        }
    }
}

void SecurityTransactionThreadFunction()
{
    for (;;)
    {
        if (event_queue.frontIsOfType(typeid(SecurityTransaction<Stock>)))
        {

            std::cout << "Stock Type in Queue: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue(); // Should move object
            SecurityTransaction<Stock> *transaction = static_cast<SecurityTransaction<Stock> *>(event.get());

            if (transaction->is_stock()) {
                std::cout << "Transaction is not a stock transaction. Skipping." << std::endl;
                continue;
            }

            std::scoped_lock<std::mutex> savingsAccountLock(databaseSavingsAccountsMutex);
            bank::SavingsAccount* savingsAccount = database.getSavingsAccountFor(transaction->getToAccount());
            if (savingsAccount == nullptr) {
                std::cout << "No savings account found for user. Cannot withdraw/deposit funds made by selling/buying stocks." << std::endl;
                continue; // lock released by exiting scope of loop.
            }

            if (transaction->getTransactionCost() > savingsAccount->getAmount()) {
                std::cout << "Not enough funds to buy stocks." << std::endl;
                continue;
            }

            std::scoped_lock<std::mutex> stockAccountLock(databaseStockAccountsMutex);
            bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>>* securityAccount = database.getStockAccountFor(transaction->getToAccount());
            if (securityAccount == nullptr) {
                std::cout << "No bond account found for user. Cannot transfer bonds." << std::endl;
                continue; // lock released by exiting scope of loop.
            }

            // This part should be strong guarantee
            try {
                // copy of each account
            auto securities = transaction->getSecurities();
            securityAccount->addSecurities(std::move(securities));

            savingsAccount->reduceAmount(transaction->getTransactionCost());
            securityAccount->addToTransactionLog(*transaction);  // std::move(transaction)
            } catch (...) {
                // revert accounts to previous state
            }
        }
        if (event_queue.frontIsOfType(typeid(SecurityTransaction<Bond>)))
        {

            std::cout << "Bond Type in Queue: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue(); // Should move object
            SecurityTransaction<Bond> *transaction = static_cast<SecurityTransaction<Bond> *>(event.get());

            if (!transaction->is_bond()) {
                std::cout << "Transaction is not a bond transaction. Skipping." << std::endl;
                continue;
            }
           
            std::scoped_lock<std::mutex> savingsAccountLock(databaseSavingsAccountsMutex);
            bank::SavingsAccount* savingsAccount = database.getSavingsAccountFor(transaction->getToAccount());
            if (savingsAccount == nullptr) {
                std::cout << "No savings account found for user. Cannot withdraw/deposit funds made by selling/buying stocks." << std::endl;
                continue; // lock released by exiting scope of loop.
            }

            if (transaction->getTransactionCost() > savingsAccount->getAmount()) {
                std::cout << "Not enough funds to buy stocks." << std::endl;
                continue;
            }

            std::scoped_lock<std::mutex> bondAccountLock(databaseBondAccountsMutex);
            bank::SecuritiesAccount<Bond, SecurityTransaction<Bond>>* securityAccount = database.getBondAccountFor(transaction->getToAccount());
            if (securityAccount == nullptr) {
                std::cout << "No bond account found for user. Cannot transfer bonds." << std::endl;
                continue; // lock released by exiting scope of loop.
            }

            // This part should be strong guarantee
            try {
                // copy of each account
            auto securities = transaction->getSecurities();
            securityAccount->addSecurities(std::move(securities));
            
            savingsAccount->reduceAmount(transaction->getTransactionCost());
            securityAccount->addToTransactionLog(*transaction);  // std::move(transaction)
            } catch (...) {
                // revert accounts to previous state
            }
        }    
    }
}

void ConversionTransactionThreadFunction()
{

    for (;;)
    {

        if (event_queue.frontIsOfType(typeid(ConversionTransaction)))
        {
            std::cout << "CONVERSION TYPE in QUEUE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue(); // Should move object
            ConversionTransaction *transaction = static_cast<ConversionTransaction *>(event.get());

            // Handle conversion
        }
    }
}

int main()
{
    bank::SavingsAccount s1(1, "Opsparingskonto", "a.jensen@gmail.com", Cash(10000, DKK));
    bank::SavingsAccount s2(2, "Opsparingskonto", "b.hansen@gmail.com", Cash(5400, DKK));
    database.savingsAccounts.emplace(s1.getUserEmail(), std::move(s1));
    database.savingsAccounts.emplace(s2.getUserEmail(), std::move(s2));

    bank::SecuritiesAccount<Stock, SecurityTransaction<Stock>> s3(1, "Aktiekonto", "a.jensen@gmail.com");
    // bank::SecuritiesAccount<Stock> s4(2, "Aktiekonto", "b.hansen@gmail.com");
    database.stockAccounts.emplace(s3.getUserEmail(), std::move(s3));
    /* database.stockAccounts.emplace(s4.getID(), std::move(s4));

     bank::SecuritiesAccount<Bond> s5(1, "Obligationskonto", "a.jensen@gmail.com");
     bank::SecuritiesAccount<Bond> s6(2, "Obligationskonto", "b.hansen@gmail.com");
     database.bondAccounts.emplace(s5.getID(), std::move(s5));
     database.bondAccounts.emplace(s6.getID(), std::move(s6));*/

    // std::unique_ptr<Event> eventRequest = std::make_unique<CashTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr
    std::thread cashTransactionThread(cashTransactionThreadFunction);
    std::thread securitiesTransactionThread(SecurityTransactionThreadFunction);

    cashTransactionThread.detach();
    securitiesTransactionThread.detach();

    // event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue

    // s1.print();
    // s2.print();

    for (auto x : database.savingsAccounts)
    {
        x.second.print();
    }

    for (auto x : database.stockAccounts)
    {
        x.second.print();
    }

    std::unique_ptr<Event> eventRequest; // Send condition on eventRequest
    std::string userEmail;
    do
    {
        std::cout << "User logged in as: ";
        std::cin >> userEmail;
        if (!database.accountExistsFor(userEmail)) {
            std::cout << "User does not exist. Please try again." << std::endl;
        }
        else {
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
            if (choice < 48 || choice > 57)   // C = {0..9}  TODO: Should be on illegal type
            {
                throw std::invalid_argument(&choice);
            }
        }
        catch (std::invalid_argument const &ex)
        {
            std::cout << "Please enter a valid choice [Integer range 0-255]";
            std::cout << "#3: " << ex.what() << '\n';
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
            eventRequest = std::make_unique<CashTransaction>(amount, userEmail); // wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest));                        // move the unique_ptr of the event to the queue
            break;
        }

        case '2': // Transfer
        {
            std::cout << "Please enter amount to transfer: ";
            std::cin >> amount;
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

            std::string transactionTypeStr;
            TransactionType transactionType;
            do
            {
                std::cout << "\nPlease enter what to do with stock (SELL/BUY): ";
                std::cin >> transactionTypeStr;
                if (transactionTypeStr == "BUY") {
                    transactionType = TransactionType::BUY;
                }
                else if (transactionTypeStr == "SELL") {
                    transactionType = TransactionType::SELL;
                }
            } while (transactionType != TransactionType::SELL && transactionType != TransactionType::BUY);
            std::cout << transactionTypeStr << amount << " " << stockName << " stocks." << std::endl;
            
            auto stockElem = availableStocks.find(stockName);
            if (stockElem == availableStocks.end()) {
                std::cout << "Stock not found." << std::endl;
                break;
            }
            Stock stock = stockElem->second;
            std::cout << "id: " << stock.getId() << std::endl;
            eventRequest = std::make_unique<SecurityTransaction<Stock>>(userEmail, stock, amount, transactionType); // wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest));                                      // move the unique_ptr of the event to the queue
            break;
        /*case 4: // Buy|Sell Bonds
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

            std::string transactionTypeStr;
            TransactionType transactionType;
            do
            {
                std::cout << "\nPlease enter what to do with bond (SELL/BUY): ";
                std::cin >> transactionTypeStr;
                transactionType = TransactionType::BUY ? transactionTypeStr == "BUY" : transactionTypeStr == "SELL";
            } while (transactionType != TransactionType::BUY || transactionType != TransactionType::SELL);

            Bond *bond = new std::deep_copy(availableBonds[bondName]);
            eventRequest = std::make_unique<BondTransaction>(bond, amount, transactionType); // wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest));                                    // move the unique_ptr of the event to the queue
            break;
        case 5:
            break;
            // Select convertFrom
            // Select convertTo and desiredAmount

            // if (cash && cash) {
            //     eventRequest = std::make_unique<CashTransaction>(-2000, s1.getID(), s2.getID());
            //     event_queue.enqueue(std::move(eventRequest));
            // }

            // eventRequest = std::make_unique<ConversionTransaction<FromType, ToType>(desiredAmount, TransactionType::CONVERT); //wrap event object in unique_ptr
            // event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            */
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

    for (auto x : database.savingsAccounts)
    {
        x.second.print();
    }
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

#include <iostream>
#include <vector>
#include <typeinfo>
// #include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
// #include "definitions/Event.hpp"
#include <memory>

DatabaseMock database;
EventMessageQueue<std::unique_ptr<Event>> event_queue; // Main event queue. Only supports Event-derived types, as Event is abstract.
                                                       // unique_ptr is used as it is not
                                                       // possible to create intances of Event
                                                       // because it is abstract

void cashTransactionThreadFunction()
{
    for (;;)
    {
        if (event_queue.frontIsOfType(typeid(CashTransaction)))
        {
            std::cout << "CASH TYPE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue(); // Should move object
            auto cash_transaction = static_cast<CashTransaction *>(event.get());

            int amount = cash_transaction->getAmount();
            std::string to = cash_transaction->getToAccount();
            std::string from = cash_transaction->getFromAccount();

            std::cout << "Transfer " << amount << " to " << to << " from " << from << std::endl;

            SavingsAccount *to_account = database.getSavingsAccountByEmail(to);
            SavingsAccount *from_account = database.getSavingsAccountByEmail(from);

            if (to_account != nullptr)
            {
                if (from_account != nullptr)
                {
                    from_account->removeAmount(amount);
                    to_account->addAmount(amount);

                    std::cout << "Transfered" << std::endl;
                }
                else
                {
                    std::cout << "Error: From account not found" << std::endl;
                }
            }
            else
            {
                std::cout << "Error: To account not found" << std::endl;
            }

            // Handle Transaction depending on if deposit, withdrawel or transfer

            // if (Transaction.from) {
            //     SavingsAccount fromSavingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
            //     fromSavingsAccount.removeAmount(transaction->getAmount());
            // }

            // Strong guarantee that amount must be added only if removed

            // SavingsAccount toSavingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
            // toSavingsAccount.addAmount(transaction->getAmount());

            // Save transaction to appropiate account
        }
    }
}

void SecurityTransactionThreadFunction()
{

    for (;;)
    {

        if (event_queue.frontIsOfType(typeid(SecurityTransaction)))
        {
            std::cout << "SECURITY TYPE in QUEUE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue(); // Should move object
            SecurityTransaction *transaction = static_cast<SecurityTransaction *>(event.get());

            // Handle Transaction
            //  Should confirm that savingsAccount exist

            // Mutex for each account type
            //{
            //    std::scoped_lock<std::mutex> lock(databaseSavingsAccountsMutex);
            //    SavingsAccount savingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
            //}
            /*
                        try
                        {
                            // Place locks around database access.
                            if (typeid(transaction) == StockTransaction)
                            {

                                {
                                    std::scoped_lock<std::mutex> lock(databaseSavingsAccountsMutex);
                                    SecuritiesAccount<Stock> securitiesAccount = database.getStockAccount(transaction->getToAccountID());
                                    SecuritiesAccount<Stock> securitiesAccountCopy = std::deep_copy(securitiesAccount);
                                    securitiesAccount.addAsset(transaction->);
                                }
                                // Should withdraw amount from savingsAccount and buy stocks with it if enough funds.
                            }
                            else
                            {
                                SecuritiesAccount<Bond> securitiesAccount = database.getBondAccount(transaction->getToAccountID());
                            }
                        }
                        catch (std::exception &e)
                        {
                            securitiesAccount = securitiesAccountCopy
                        }

                        // Strong guarantee exception here if addAsset fails, we should not deduct amount but nothing should happen

                        savingsAccount.removeAmount(transaction->getAmount());
                        SecuritiesAccount.addToTransactionLog(transaction);
                        */
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
    SavingsAccount s1(1, "Opsparingskonto", "a.jensen@gmail.com", Cash(10000, DKK));
    SavingsAccount s2(2, "Opsparingskonto", "b.hansen@gmail.com", Cash(5400, DKK));
    database.savingAccounts.emplace(s1.getID(), std::move(s1));
    database.savingAccounts.emplace(s2.getID(), std::move(s2));

    /* SecuritiesAccount<Stock> s3(1, "Aktiekonto", "a.jensen@gmail.com");
     SecuritiesAccount<Stock> s4(2, "Aktiekonto", "b.hansen@gmail.com");
     database.stockAccounts.emplace(s3.getID(), std::move(s3));
     database.stockAccounts.emplace(s4.getID(), std::move(s4));

     SecuritiesAccount<Bond> s5(1, "Obligationskonto", "a.jensen@gmail.com");
     SecuritiesAccount<Bond> s6(2, "Obligationskonto", "b.hansen@gmail.com");
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

    for (auto x : database.savingAccounts)
    {
        x.second.print();
    }

    std::unique_ptr<Event> eventRequest; // Send condition on eventRequest
    std::string userEmail;

    do
    {
        std::cout << "User logged in as: ";
        std::cin >> userEmail;
        // if (!database.accountExistsFor(userEmail))
        //{
        //     std::cout << "User does not exist. Please try again." << std::endl;
        // }
        // else
        //{
        database.displayAccountsFor(userEmail);
        break;
        //}
    } while (1);

    char choice = 0;
    do
    {
        std::cout << "Please enter action 1 (Make deposit), 2 (Transfer), 3 (Trade Stocks), 4 (Trade Bonds), 5 (Convert Assets), 6 (Show accounts)" << std::endl;
        try
        {
            std::cin.ignore();
            std::cin >> choice; // TODO: Exception to handle invalid arguments
            if (choice < 0 || choice > 255)
            {
                throw std::invalid_argument(&choice);
            }
        }
        catch (std::invalid_argument const &ex)
        {
            std::cout << "Please enter a valid choice [Integer range 0-255]";
            std::cout << "#3: " << ex.what() << '\n';
            choice = 0;
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
        /*case 3: // Buy|Sell Stocks
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
                transactionType = TransactionType::BUY ? transactionTypeStr == "BUY" : transactionTypeStr == "SELL";
            } while (transactionType != TransactionType::BUY || transactionType != TransactionType::SELL);

            Stock *stock = new std::deep_copy(availableStocks[stockName]);
            eventRequest = std::make_unique<StockTransaction>(stock, amount, transactionType); // wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest));                                      // move the unique_ptr of the event to the queue
            break;*/
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
        case 6:
            database.displayAccountsFor(userEmail);
            break;
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
    //     s1.removeAmount(cashTransaction->getAmount());
    //     s2.addAmount(cashTransaction->getAmount());
    // }

    for (auto x : database.savingAccounts)
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

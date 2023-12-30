#include <iostream>
#include <vector>
#include <typeinfo>
//#include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
// #include "definitions/Event.hpp"
#include <memory>

DatabaseMock database;
EventMessageQueue<std::unique_ptr<Event>> event_queue; // Main event queue. Only supports Event-derived types, as Event is abstract.
                                                      // unique_ptr is used as it is not
                                                      // possible to create intances of Event
                                                      // because it is abstract

void cashTransactionThreadFunction() {

    for (;;) {

        if (event_queue.frontIsOfType(typeid(CashTransaction))) {
            std::cout << "CASH TYPE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue();  // Should move object 
            CashTransaction* transaction = static_cast<CashTransaction*>(event.get());

            // Handle Transaction depending on if deposit, withdrawel or transfer

            if Transaction.from {
                SavingsAccount fromSavingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
                fromSavingsAccount.removeAmount(transaction->getAmount());
            }

            // Strong guarantee that amount must be added only if removed

            SavingsAccount toSavingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
            toSavingsAccount.addAmount(transaction->getAmount());

            // Save transaction to appropiate account

        }
    }
}

void SecurityTransactionThreadFunction() {

    for (;;) {

        if (event_queue.frontIsOfType(typeid(SecurityTransaction))) {
            std::cout << "SECURITY TYPE in QUEUE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue();  // Should move object 
            SecurityTransaction* transaction = static_cast<SecurityTransaction*>(event.get());

            // Handle Transaction
            //  Should confirm that savingsAccount exist

            SavingsAccount savingsAccount = database.getSavingsAccount(transaction->getFromAccountID());
            if (typeid(transaction) == StockTransaction) {
                SecuritiesAccount<Stock> securitiesAccount = database.getStockAccount(transaction->getToAccountID());
                securitiesAccount.addAsset(transaction->);

            // Should withdraw amount from savingsAccount and buy stocks with it if enough funds.
            } else {
                SecuritiesAccount<Bond> securitiesAccount = database.getBondAccount(transaction->getToAccountID());
            }

            // Strong guarantee exception here if addAsset fails, we should not deduct amount but nothing should happen

            savingsAccount.removeAmount(transaction->getAmount());

            // Move transaction to appropriate account to log it.
        }
    }
}

void ConversionTransactionThreadFunction() {

    for (;;) {

        if (event_queue.frontIsOfType(typeid(ConversionTransaction))) {
            std::cout << "CONVERSION TYPE in QUEUE: " << event_queue.size() << std::endl;
            std::unique_ptr<Event> event = event_queue.dequeue();  // Should move object 
            ConversionTransaction* transaction = static_cast<ConversionTransaction*>(event.get());

            // Handle conversion


        }
    }

}


int main()
{
    SavingsAccount s1(1, "Opsparingskonto", "a.jensen@gmail.com", Cash(10000, DKK));
    SavingsAccount s2(2, "Opsparingskonto", "b.hansen@gmail.com", Cash(5400, DKK));    
    database.savingAccounts.insert(std::pair<int, SavingsAccount>{s1.getID(), s1});
    database.savingAccounts.insert(std::pair<int, SavingsAccount>{s2.getID(), s2});

    SecuritiesAccount<Stock> s3(1, "Aktiekonto", "a.jensen@gmail.com");
    SecuritiesAccount<Stock> s4(2, "Aktiekonto", "b.hansen@gmail.com");
    database.stockAccounts.insert(std::pair<int, SecuritiesAccount<Stock>>{s3.getID(), s3});
    database.stockAccounts.insert(std::pair<int, SecuritiesAccount<Stock>>{s4.getID(), s4});

    SecuritiesAccount<Bond> s5(1, "Obligationskonto", "a.jensen@gmail.com");
    SecuritiesAccount<Bond> s6(2, "Obligationskonto", "b.hansen@gmail.com");
    database.bondAccounts.insert(std::pair<int, SecuritiesAccount<Bond>>{s5.getID(), s5});
    database.bondAccounts.insert(std::pair<int, SecuritiesAccount<Bond>>{s6.getID(), s6});    

    // std::unique_ptr<Event> eventRequest = std::make_unique<CashTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr
    std::thread cashTransactionThread (cashTransactionThreadFunction);
    std::thread securitiesTransactionThread (SecurityTransactionThreadFunction);

    cashTransactionThread.detach();
    securitiesTransactionThread.detach();

    // event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue

    s1.print();
    s2.print();

    std::unique_ptr<Event> eventRequest;  // Send condition on eventRequest
    std::string userEmail;

    do {
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
    do {
        std::cout << "Please enter action 1 (Make deposit), 2 (Buy Stock) or 3 (Sell Stock)" << std::endl;
        try {
            std::cin.ignore();
            std::cin >> choice;  // TODO: Exception to handle invalid arguments
            if (choice < 0 || choice > 255) {
                throw std::invalid_argument(&choice);
            }
        }
        catch (std::invalid_argument const& ex) {
            std::cout << "Please enter a valid choice [Integer range 0-255]";
            std::cout << "#3: " << ex.what() << '\n';
            choice = 0;
        }

        switch (choice) // Assumes that Client (this) has no access to Accounts for now, so any mistake will be handled silently
        // in the respective thread. Client should return a Future to a list of requests, that the customer can then access 
        {
        case 1:
            eventRequest = std::make_unique<CashTransaction>(2000, s1.getID()); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;

            // Make deposit
        case 2:
            eventRequest = std::make_unique<CashTransaction>(-2000, s1.getID()); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;
            // Withdraw cash
            break;
        case 3:

            // Select stock
            for (auto &stock : availableStocks) {
                stock.second.print();
            }
            Stock* stock = new Stock(); // We assume that stocks can be "created" to a start
            const int amount = 4;
            eventRequest = std::make_unique<StockTransaction>(stock, amount, TransactionType::BUY); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;
        case 4:
            Stock* stock = new Stock();
            const int amount = 4;
            eventRequest = std::make_unique<StockTransaction>(stock, amount, TransactionType::SELL); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;
        case 5:
            // Select Bond
            
            const int amount = 4;
            std::vector<Bond> bonds = {Bond(), Bond(), Bond(), Bond()};
            
            eventRequest = std::make_unique<BondTransaction>(&bonds, TransactionType::BUY); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;
        case 6:
            Bond* bond = new Bond(); // Move entity
            const int amount = 4;
            eventRequest = std::make_unique<BondTransaction>(bond, amount, TransactionType::SELL); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;
        case 7:

            // Select convertFrom
            // Select convertTo and desiredAmount
            
            if (cash && cash) {
                eventRequest = std::make_unique<CashTransaction>(-2000, s1.getID(), s2.getID());
                event_queue.enqueue(std::move(eventRequest)); 
            }

            //eventRequest = std::make_unique<ConversionTransaction<FromType, ToType>(desiredAmount, TransactionType::CONVERT); //wrap event object in unique_ptr
            //event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            
        case 8: 
            database.displayAccountsFor(userEmail);
            break;
        default:
            break;
        }
    } while (choice >= 0);




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

    s1.print();
    s2.print();
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

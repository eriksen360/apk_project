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
            std::cout << "CASH TYPE\n";
            std::unique_ptr<Event> event = event_queue.dequeue();  // Should move object 
            CashTransaction* transaction = static_cast<CashTransaction*>(event.get());

            // Handle Transaction
        }
    }
}

void SecurityTransactionThreadFunction() {

    for (;;) {

        if (event_queue.frontIsOfType(typeid(SecurityTransaction))) {
            std::cout << "SECURITY TYPE\n";
            std::unique_ptr<Event> event = event_queue.dequeue();  // Should move object 
            SecurityTransaction* transaction = static_cast<SecurityTransaction*>(event.get());

            // Handle Transaction
        }
    }
}


int main()
{
    SavingsAccount s1(1, "Forbrugskonto", Cash(10000, DKK));
    SavingsAccount s2(2, "Opsparingskonto", Cash(20000, DKK));
    //SecuritiesAccount<StockTransaction> s3(1, "Aktiekonto");
    database.savingAccounts.insert(std::pair<int, SavingsAccount>{s1.getID(), s1});
    database.savingAccounts.insert(std::pair<int, SavingsAccount>{s2.getID(), s2});
    //database.stockAccounts.insert(std::pair<int, SecuritiesAccount<StockTransaction>>{s3.getID(), s3});

    // std::unique_ptr<Event> eventRequest = std::make_unique<CashTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr

    std::thread cashTransactionThread (cashTransactionThreadFunction);
    std::thread securitiesTransactionThread (SecurityTransactionThreadFunction);

    cashTransactionThread.detach();
    securitiesTransactionThread.detach();

    // event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue

    s1.print();
    s2.print();

    // Start tråde her
    std::unique_ptr<Event> eventRequest;  // Send condition on eventRequest
    uint choice = 0; 
    do {
        std::cout << "Please enter action 1 (Make deposit), 2 (Buy Stock) or 3 (Sell Stock)" << std::endl;
        std::cin >> choice;  // TODO: Exception to handle invalid arguments

        switch (choice) // Assumes that Client (this) has no access to Accounts for now, so any mistake will be handled silently
        // in the respective thread.

        // Client should return a Future to a list of requests, that the customer can then access 
        {
        case 1:
            // Get amount 
            // Enqueue a CashTransaction
            eventRequest = std::make_unique<CashTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;
        case 2:

            // Select Stock

            //eventRequest = std::make_unique<SecurityTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr
            //event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            break;
        case 3:
            // Select Stock
            // Enqueue a StockTransaction
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

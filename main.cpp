#include <iostream>
#include <vector>
// #include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
// #include "definitions/Event.hpp"
#include <memory>

int main()
{
    MessageQueue<std::unique_ptr<Event>> event_queue; // Main event queue. 
                                                      // unique_ptr is used as it is not
                                                      // possible to create intances of Event
                                                      // because it is abstract

    SavingsAccount s1(1, "Forbrugskonto", DKK, 10000);
    SavingsAccount s2(2, "Opsparingskonto", DKK, 20000);

    std::unique_ptr<Event> eventRequest = std::make_unique<CashTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr

    event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue

    s1.print();
    s2.print();

    auto eventHandler = event_queue.dequeue();  // get the unique_ptr of the event from the queue

    // Not possible to call CashTransaction specific function without
    // casting the Event ptr to CashTransaction ptr. 
    // Should be used with caution, as it is done runtime
    // Skal laves om til exception hvis den fejler
    if (auto cashTransaction = static_cast<CashTransaction *>(eventHandler.get()))
    {
        s1.removeAmount(cashTransaction->getAmount());
        s2.addAmount(cashTransaction->getAmount());
    }

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

#include <iostream>
#include <vector>
#include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
#include "definitions/Event.hpp"

int main() {
    MessageQueue<Event> event_queue; // Main event queue

    SavingsAccount s1(1, "Forbrugskonto", DKK, 10000);
    SavingsAccount s2(2, "Opsparingskonto", DKK, 20000);

    CashTransaction ct1(2000, s1.getID(), s2.getID());

    event_queue.enqueue(ct1);
    
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

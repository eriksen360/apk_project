#include <iostream>
#include <vector>
#include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
#include "definitions/Event.hpp"

int main() {
    MessageQueue<Event> event_queue;

    Bond* bond = new Bond();
    std::cout << bond->foo();

    std::vector<StockAccount> accounts; 
    
    delete bond;
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

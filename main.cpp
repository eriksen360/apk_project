#include <iostream>
#include <vector>
//#include "definitions/Asset.hpp"
#include "definitions/Account.hpp"
#include "definitions/Queue.hpp"
// #include "definitions/Event.hpp"
#include <memory>
#include "Threads.cpp"

pthread_cond_t cashTransactionCond;
pthread_cond_t securityTransactionCond;

bool queueEmpty = true;

pthread_mutex_t accountsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t eventQueueMutex = PTHREAD_MUTEX_INITIALIZER;

DatabaseMock* database = new DatabaseMock();
MessageQueue<std::unique_ptr<Event>> event_queue; // Main event queue. -> Why not for each EventType?
                                                      // unique_ptr is used as it is not
                                                      // possible to create intances of Event
                                                      // because it is abstract

void *SavingsThreadFunction(void* arg) {

    /*
        If event is enqueued or queue is not empty we should get
        mutex, check if element in front is type we should grab
        and then if match dequeue it to thread.

        EventEnqueued is a thread condition
        We have a QueueThread which has the purpose of monitoring the queue and marking it empty or not for the other queues.
    
    */

    




    // Dequeue from MockDB -> Use Type of Event to deque

    //pthread_mutex_lock(&eventQueueMutex);

    

    // Wait for condition set by by client or queue?
    auto eventHandler = event_queue.dequeue();  // get the unique_ptr of the event from the queue
    
    // If event extract by move, unlock and handle. Hvorfor ikke en kø til hver type?
    pthread_mutex_unlock(&eventQueueMutex);

    // Do something with Event

    // Wait for DB connection to open

    // Update Account element 

    // release database connection
}

void *SecuritiesThreadFunction(void* arg) {

}


int main()
{
    pthread_t savingsThread;
    pthread_t securitiesThread;

    SavingsAccount s1(1, "Forbrugskonto", DKK, 10000);
    SavingsAccount s2(2, "Opsparingskonto", DKK, 20000);
    database->savingAccounts.insert(std::pair<int, SavingsAccount>{1, s1});
    database->savingAccounts.insert(std::pair<int, SavingsAccount>{2, s2});

    // std::unique_ptr<Event> eventRequest = std::make_unique<CashTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr

    pthread_create(&savingsThread, NULL, &SavingsThreadFunction, NULL);
    pthread_create(&securitiesThread, NULL, &SecuritiesThreadFunction, NULL);

    // event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue

    s1.print();
    s2.print();

    // Start tråde her
    std::unique_ptr<Event> eventRequest;  // Send condition on eventRequest
    uint choice = 0; 
    do {
        std::cout << "Please enter action 1 (Make deposit), 2 (Buy Stock) or 3 (Sell Stock)" << std::endl;
        std::cin >> choice;

        switch (choice) // Assumes that Client (this) has no access to Accounts for now, so any mistake will be handled silently
        // in the respective thread.

        // Client should return a Future to a list of requests, that the customer can then access 
        {
        case 0:
            
        case 1:
            // Get amount 
            // Enqueue a CashTransaction
            eventRequest = std::make_unique<CashTransaction>(2000, s1.getID(), s2.getID()); //wrap event object in unique_ptr
            event_queue.enqueue(std::move(eventRequest)); // move the unique_ptr of the event to the queue
            pthread_cond_signal(&cashTransactionCond);
            break;
        case 2:
            // Select Stock
            // Enqueue a StockTransaction
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

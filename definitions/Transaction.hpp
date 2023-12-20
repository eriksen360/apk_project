#include "Event.hpp"

template<typename T>
class Transaction : Event {
private:
    int id;  // TODO: Use boost library UUID as Id
    int fromAccountId;
    int toAccountId;

    T fromTransactionType; // trait


public:
    Transaction() {};
    int getEventId() { return id; }
    // virtual
    virtual ~Transaction() {};


};


void bar<StockTransaction> {

    // gør 1 ting
}


void bar<BondTransaction> {

    // gør 1 anden ting
}




class CashTransaction : Transaction {
private:
    int amount;
public:
    CashTransaction(int amount) {
        this->amount = amount;
    }
    int getTransactionAmount() { return amount; };
};

class StockTransaction : Transaction {

};

class BondTransaction : Transaction {

};

template<typename T, typename U>  // T1 and T2 must be of derived type Transaction
class ConversionTransaction : Transaction {
private:
    T from;
    U to;
public:
    U convert(T from, U to);  // Static?
};



/* 
    Concept til at gøre is_convertible_from_and_to et trait på alle transaktiontyper

    Stock og Bond er ikke konverterbare intern, men begge konverterbare til Cash
    Cash er konverterbar til begge typer.
*/

#include "Event.hpp"

class Transaction : public Event
{
public:
    Transaction(int from, int to) : Event(), fromAccountId(from), toAccountId(to) {}
    virtual ~Transaction(){};

private:
    int fromAccountId;
    int toAccountId;
};

class CashTransaction : public Transaction
{
public:
    CashTransaction(int amount, int from, int to) : Transaction(from, to), amount(amount) {}
    int getTransactionAmount() { return amount; };

    void createID() override {
        setID(123); // TODO: make correct implementation
    }

    int getAmount() const{
        return amount;
    }

private:
    int amount;
};

// class StockTransaction : Transaction
// {
// };

// class BondTransaction : Transaction
// {
// };

// template <typename T, typename U> // T1 and T2 must be of derived type Transaction
// class ConversionTransaction : Transaction
// {
// private:
//     T from;
//     U to;

// public:
//     U convert(T from, U to); // Static?
// };
/*
    Concept til at gøre is_convertible_from_and_to et trait på alle transaktiontyper

    Stock og Bond er ikke konverterbare intern, men begge konverterbare til Cash
    Cash er konverterbar til begge typer.
*/

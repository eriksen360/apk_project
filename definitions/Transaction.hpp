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

// Transactions requires std::constness of type -> Queue should contain * to const Events

class SecurityTransaction : public Transaction {
public:
    virtual void buyAsset() = 0;
    virtual void sellAsset() = 0;
};

class BondTransaction : public SecurityTransaction
{
private:
    std::vector<Bond> bonds;
};

class StockTransaction : public SecurityTransaction
{
private:
    std::vector<Stock> stocks;
};

template <typename T, typename U> 
requires asset_is_convertible<T, U>
class ConversionTransaction : Transaction
{
private:
    T from;
    U to;
public:
    U convert(T from, U to);
};




/*
    Concept til at gøre is_convertible_from_and_to et trait på alle transaktiontyper

    Stock og Bond er ikke konverterbare intern, men begge konverterbare til Cash
    Cash er konverterbar til begge typer.
*/

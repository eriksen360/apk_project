#include "Event.hpp"
#include "Asset.hpp"

enum TransactionType
{
    BUY = 0,
    SELL = 1,
    CONVERT = 2
};

class Transaction : public Event
{
public:
    Transaction(std::string to, std::string from) : Event(), toAccountEmail(to), fromAccountEmail(from){};
    Transaction(std::string to) : Event(), toAccountEmail(to), fromAccountEmail(0){};
    virtual ~Transaction(){};

    std::string getToAccount()
    {
        return toAccountEmail;
    }

    std::string getFromAccount()
    {
        return fromAccountEmail;
    }

private:
    std::string toAccountEmail;
    std::string fromAccountEmail;
};

class CashTransaction : public Transaction
{
public:
    CashTransaction(int amount, std::string to, std::string from) : Transaction(to, from), amount(amount) {}
    CashTransaction(int amount, std::string to) : Transaction(to), amount(amount) {}
    int getTransactionAmount() { return amount; };

    void createID() override
    {
        setID(123); // TODO: make correct implementation
    }

    int getAmount() const
    {
        return amount;
    }

private:
    int amount;
};

// Transactions requires std::constness of type -> Queue should contain * to const Events

class SecurityTransaction : public Transaction
{
public:
    SecurityTransaction(std::string to, std::string from) : Transaction(to, from){};
};

class BondTransaction : public SecurityTransaction
{
private:
    std::vector<Bond> bonds;
    TransactionType type;

public:
    BondTransaction(std::string to, std::string from, std::vector<Bond> &bonds, TransactionType type)
        : SecurityTransaction(to, from), bonds(std::move(bonds)), type(type) {}

    TransactionType getType() const
    {
        return type;
    }

    size_t getSize() const
    {
        return bonds.size();
    }

    ~BondTransaction() override {}
};

class StockTransaction : public SecurityTransaction
{
private:
    std::vector<Stock> stocks;
    TransactionType type;

public:
    StockTransaction(std::string to, std::string from, std::vector<Stock> &stocks, TransactionType type)
        : SecurityTransaction(to, from), stocks(std::move(stocks)), type(type) {}

    TransactionType getType() const
    {
        return type;
    }

    size_t getSize() const
    {
        return stocks.size();
    }

    ~StockTransaction() override {}
};

class ConversionTransaction : public Transaction
{
};

/*
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

*/

/*
    Concept til at gøre is_convertible_from_and_to et trait på alle transaktiontyper

    Stock og Bond er ikke konverterbare intern, men begge konverterbare til Cash
    Cash er konverterbar til begge typer.
*/

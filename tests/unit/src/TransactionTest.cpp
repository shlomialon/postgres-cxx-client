#include <gtest/gtest.h>
#include <postgres/Connection.h>
#include <postgres/Error.h>
#include <postgres/Transaction.h>

namespace postgres {

inline auto constexpr CREATE = "CREATE TEMP TABLE tx_test (val INT)";
inline auto constexpr INSERT = "INSERT INTO tx_test (val) VALUES (1)";
inline auto constexpr SELECT = "SELECT val FROM tx_test";

TEST(TransactionTest, Ok) {
    Connection conn{};
    conn.exec(CREATE);
    conn.transact(INSERT, INSERT);
    ASSERT_EQ(2, conn.exec(SELECT).size());
}

TEST(TransactionTest, Bad) {
    Connection conn{};
    conn.exec(CREATE);
    ASSERT_THROW(conn.transact(INSERT, "BAD"), RuntimeError);
    ASSERT_EQ(0, conn.exec(SELECT).size());
}

TEST(TransactionTest, Commit) {
    Connection conn{};
    conn.exec(CREATE);
    auto tx = conn.begin();
    conn.exec(INSERT);
    tx.commit();
    ASSERT_EQ(1, conn.exec(SELECT).size());
}

TEST(TransactionTest, Rollback) {
    Connection conn{};
    conn.exec(CREATE);
    {
        auto tx = conn.begin();
        conn.exec(INSERT);
    }
    ASSERT_EQ(0, conn.exec(SELECT).size());
}

TEST(TransactionTest, Misuse) {
    Connection conn{};
    auto       tx = conn.begin();
    tx.commit();
    ASSERT_THROW(tx.commit(), LogicError);
}

TEST(TransactionTest, Move) {
    Connection conn{};
    auto       tx  = conn.begin();
    auto       tx2 = std::move(tx);
    ASSERT_THROW(tx.commit(), LogicError);
    tx2.commit();
}

}  // namespace postgres

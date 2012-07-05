//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Test suite case for the rdb-mysql-query-test.
//
//------------------------------------------------------------------------

//---------------
// C++ Headers --
//---------------
#include "RdbMySQL/Client.h"
#include "RdbMySQL/Conn.h"
#include "RdbMySQL/Query.h"
#include "RdbMySQL/Result.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

using namespace RdbMySQL;
using namespace std;

#define BOOST_TEST_MODULE rdb-mysql-query-test
#include <boost/test/included/unit_test.hpp>

/**
 * Simple test suite for module rdb-mysql-query-test.
 * See http://www.boost.org/doc/libs/1_36_0/libs/test/doc/html/index.html
 */

namespace {


class QueryTest : public Query {
public:

  /**
   *  Constructor takes connection object
   *
   *  @param conn   database connection object
   */
  QueryTest ( Conn& conn ) : Query(conn) {}


  // Destructor
  virtual ~QueryTest () {}

  /**
   *  Execute complete query. Different signatures, all about efficiency.
   */
  virtual Result* execute ( const char* q ) ;
  virtual Result* execute ( const std::string& q ) ;
  virtual Result* execute ( const char* q, size_t size ) ;

  const std::string& fullQuery() const { return m_fullQuery; }

private:

  std::string m_fullQuery;

} ;

Result*
QueryTest::execute ( const char* q )
{
  m_fullQuery = q;
  cout << "'" << m_fullQuery << "'" << endl ;
  return 0 ;
}

Result*
QueryTest::execute ( const char* q, size_t size )
{
  m_fullQuery = std::string(q,size);
  cout << "'" << m_fullQuery << "'" << endl ;
  return 0 ;
}

Result*
QueryTest::execute ( const std::string& q )
{
  m_fullQuery = q;
  cout << "'" << m_fullQuery << "'" << endl ;
  return 0 ;
}

}

// ==============================================================

BOOST_AUTO_TEST_CASE( test_1 )
{
  // make a conection object
  Conn conn("psdb", "regdb_reader", "", "regdb") ;

  // open connection
  BOOST_REQUIRE(conn.open());

  // make a query object
  QueryTest query( conn ) ;

  // do some queries
  std::auto_ptr<Result> res ;
  const char* q ;

  q = "SELECT * FROM ATABLE" ;
  res.reset ( query.execute(q) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "SELECT * FROM ATABLE");

  q = "SELECT * FROM ?" ;
  res.reset ( query.executePar( q, "ATABLE" ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "SELECT * FROM ATABLE");

  q = "SELECT * FROM ? WHERE a = ?? AND b = ??" ;
  res.reset ( query.executePar( q, "ATABLE", 1, "aqua\n" ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "SELECT * FROM ATABLE WHERE a = 1 AND b = 'aqua\\n'");

  q = "INSERT INTO ? ( strval, boolval ) VALUES ( ??, ?? )" ;
  res.reset ( query.executePar( q, "ATABLE", "aqua\n", true ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( strval, boolval ) VALUES ( 'aqua\\n', 1 )");

  q = "INSERT INTO ? ( chval, schval, uchval ) VALUES ( ??, ??, ?? )" ;
  res.reset ( query.executePar( q, "ATABLE", char('c'), (signed char)('c'), (unsigned char)('c') ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( chval, schval, uchval ) VALUES ( 'c', 99, 99 )");
  res.reset ( query.executePar( q, "ATABLE", char('�'), (signed char)('�'), (unsigned char)('�') ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( chval, schval, uchval ) VALUES ( '�', -5, 251 )");

  q = "INSERT INTO ? ( shrt, ushrt ) VALUES ( ??, ?? )" ;
  res.reset ( query.executePar( q, "ATABLE", (short)(1), (unsigned short)(1) ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( shrt, ushrt ) VALUES ( 1, 1 )");
  res.reset ( query.executePar( q, "ATABLE", (short)(~0), (unsigned short)(~0) ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( shrt, ushrt ) VALUES ( -1, 65535 )");

  q = "INSERT INTO ? ( int, uint ) VALUES ( ??, ?? )" ;
  res.reset ( query.executePar( q, "ATABLE", (int)(1), (unsigned int)(1) ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( int, uint ) VALUES ( 1, 1 )");
  res.reset ( query.executePar( q, "ATABLE", (int)(~0), (unsigned int)(~0) ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( int, uint ) VALUES ( -1, 4294967295 )");

  q = "INSERT INTO ? ( long, ulong ) VALUES ( ??, ?? )" ;
  res.reset ( query.executePar( q, "ATABLE", 1L, (unsigned long)(1L) ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( long, ulong ) VALUES ( 1, 1 )");
  res.reset ( query.executePar( q, "ATABLE", ~0L, (unsigned long)(~0L) ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( long, ulong ) VALUES ( -1, 18446744073709551615 )");

  q = "INSERT INTO ? ( flt, dbl, ldbl ) VALUES ( ??, ??, ?? )" ;
  res.reset ( query.executePar( q, "ATABLE", float(2./3.), 2./3., (long double)2./3. ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( flt, dbl, ldbl ) VALUES ( 0.6666667, 0.6666666666666666, 0.6666666666666666667 )");

  q = "INSERT INTO ? ( dbl1, dbl2, dbl3 ) VALUES ( ??, ??, ?? )" ;
  res.reset ( query.executePar( q, "ATABLE", 2./3e30, 2./3., 2./3e-30 ) ) ;
  BOOST_CHECK_EQUAL(query.fullQuery(), "INSERT INTO ATABLE ( dbl1, dbl2, dbl3 ) VALUES ( 6.666666666666668e-31, 0.6666666666666666, 6.666666666666666e+29 )");
}

// ==============================================================

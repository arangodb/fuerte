#include <fuerte/connection.h>
#include <fuerte/database.h>
#include <boost/algorithm/string.hpp>

namespace arangocxx {
  using namespace arangocxx::detail;
  Database::Database(std::shared_ptr<Connection> conn, std::string name)
    : _conn(conn)
    , _name(name)
    {
      //get or create db via network
    }


}


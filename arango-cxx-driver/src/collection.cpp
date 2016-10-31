#include <fuerte/collection.h>
#include <fuerte/connection.h>
#include <fuerte/database.h>
#include <boost/algorithm/string.hpp>

namespace arangocxx {
  using namespace arangocxx::detail;
  Collection::Collection(std::shared_ptr<Database> conn, std::string name)
    : _db(conn)
    , _name(name)
    {
    }


}


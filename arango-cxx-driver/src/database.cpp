#include <fuerte/database.h>
#include <fuerte/collection.h> //required by new
#include <fuerte/connection.h> //required by _conn

namespace arangodb { namespace fuerte { inline namespace v1 {

  using namespace arangodb::fuerte::detail;

  Database::Database(std::shared_ptr<Connection> conn, std::string const& name)
    : _conn(conn)
    , _name(name)
    {}

  std::shared_ptr<Collection> Database::getCollection(std::string const& name){
    return std::shared_ptr<Collection>( new Collection(shared_from_this(),name) );
  }

  std::shared_ptr<Collection> Database::createCollection(std::string const& name){
    return std::shared_ptr<Collection>( new Collection(shared_from_this(),name) );
  }
  bool Database::deleteCollection(std::string const& name){
    return false;
  }

}}}

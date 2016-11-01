#include <fuerte/next/database.h>   // datbase and collection need each other - how to resolve the cycle
#include <fuerte/next/collection.h> //required by new
#include <fuerte/next/connection.h> //required by _conn

namespace arangocxx {

  using namespace arangocxx::detail;

  Database::Database(std::shared_ptr<Connection> conn, std::string name)
    : _conn(conn)
    , _name(name)
    {
    }

  std::shared_ptr<Collection> Database::getCollection(std::string name){
    return std::shared_ptr<Collection>( new Collection(shared_from_this(),name) );
  }

  std::shared_ptr<Collection> Database::createCollection(std::string name){
    return std::shared_ptr<Collection>( new Collection(shared_from_this(),name) );
  }
  bool Database::deleteCollection(std::string name){
    return false;
  }

}


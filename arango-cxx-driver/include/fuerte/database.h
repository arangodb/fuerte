#pragma once
#ifndef ARANGO_CXX_DRIVER_DATABASE
#define ARANGO_CXX_DRIVER_DATABASE

#include <memory>
#include <string>

namespace arangodb { namespace fuerte { inline namespace v1 {

class Connection;
class Collection;

class Database : public std::enable_shared_from_this<Database> {
  friend class Connection;

  public:
    std::shared_ptr<Collection> getCollection(std::string name);
    std::shared_ptr<Collection> createCollection(std::string name);
    bool deleteCollection(std::string name);

  private:
    Database(std::shared_ptr<Connection>, std::string name);
    std::shared_ptr<Connection> _conn;
    std::string _name;
};

}}}
#endif

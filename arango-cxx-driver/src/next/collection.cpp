#include <fuerte/next/collection.h>
#include <fuerte/next/database.h>

namespace arangodb { namespace rest { inline namespace v2 {

  using namespace arangodb::rest::detail;

  Collection::Collection(std::shared_ptr<Database> db, std::string name)
    : _db(db)
    , _name(name)
    {}

}}}

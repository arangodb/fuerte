#include <fuerte/collection.h>
#include <fuerte/database.h>

namespace arangodb { namespace fuerte { inline namespace v1 {

  using namespace arangodb::fuerte::detail;

  Collection::Collection(std::shared_ptr<Database> db, std::string name)
    : _db(db)
    , _name(name)
    {}

}}}

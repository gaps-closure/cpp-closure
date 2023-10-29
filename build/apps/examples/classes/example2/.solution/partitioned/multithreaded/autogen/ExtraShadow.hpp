#include "Parent.hpp"
#include "ClosureRemoteHalMaster.hpp"

// This is Extra's shadow 
class Extra : public Parent
{
private:
  int oid;

public:
  Extra() {
    oid = ClosureRemoteHalMaster::instantiateExtra();
  }

  // @PurpleOrangeCallable
  int getValue() {
    return ClosureRemoteHalMaster::invokeExtraGetValue(oid);
  }
};


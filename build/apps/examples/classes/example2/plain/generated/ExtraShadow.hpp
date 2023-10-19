#include "Parent.hpp"
#include "remote/ClosureRemoteHalMaster.hpp"

// global in Orange
int global_oid = 1;

// This is Extra's shadow 
class Extra : public Parent
{
private:
  int oid;

public:
  Extra() {
    oid = global_oid++;
    ClosureRemoteHalMaster::instantiateExtra();
  }

  // @PurpleOrangeCallable
  int getValue() {
    return ClosureRemoteHalMaster::invokeExtraGetValue(oid);
  }
};


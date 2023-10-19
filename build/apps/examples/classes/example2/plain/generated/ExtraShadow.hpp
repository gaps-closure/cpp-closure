#include "Parent.hpp"

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
    // - call to serialize a constructor call
    // - serialize and send it to Purple
  }

  // @PurpleOrangeCallable
  int getValue() {
    // - call to serialize a method call
    // - serialize and send it to Purple
    // - return the value
    // return lib(oid, method_name, vector<args>)
    return 0; // TODO
  }
};


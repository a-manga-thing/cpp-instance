#include "BaseCtrl.h"
#include "Util.h"

bool BaseCtrl::validate(HttpCallback& callback, CJPR jsonPtr)
{
    std::string err;
    
    if(!jsonPtr) {
        badRequest(callback, "No json object is found in the request");
        return false;
    }
    
    if(!doCustomValidations(*jsonPtr, err)) {  //TODO: add json validator
        badRequest(callback, err);  //for all the required fields
        return false;
    }
    
    return true;
}

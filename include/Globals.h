#pragma once

#include "Types.h"
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>

class Globals {
    std::unordered_map<std::string, Instance> followers;
    std::shared_mutex followersMutex;
    
    std::unordered_map<std::string, Instance> following;
    std::shared_mutex followingMutex;
    
    std::unordered_set<std::string> tryFollow;
    std::shared_mutex tryFollowMutex;
    
public:
    Instance instance;
    
    bool inFollowers(CSR str);
    bool inFollowing(CSR str);
    bool inTryFollow(CSR str);
    
    const Instance& getFollowing(CSR id);
    
    const Instance& addFollower(CSR publicKey, CSR id);
    const Instance& addFollowing(CSR id);
};

extern Globals globals;

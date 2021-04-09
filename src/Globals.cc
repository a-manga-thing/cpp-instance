#include "Globals.h"
#include "Types.h"
#include "Util.h"

bool Globals::inFollowers(CSR str)
{
    followersMutex.lock_shared();
    bool ret = followers.contains(str);
    followersMutex.unlock_shared();
    return ret;
}

bool Globals::inFollowing(CSR str)
{
    followingMutex.lock_shared();
    bool ret = following.contains(str);
    followingMutex.unlock_shared();
    return ret;
}

bool Globals::inTryFollow(CSR str)
{
    tryFollowMutex.lock_shared();
    bool ret = tryFollow.contains(str);
    tryFollowMutex.unlock_shared();
    return ret;
}

const Instance& Globals::addFollower(CSR publicKey, CSR id)
{
    Instance temp = {id, publicKey, ""};
    followersMutex.lock();
    followers.emplace(std::make_pair(temp.url, temp));
    followersMutex.unlock();
}

const Instance& Globals::addFollowing(CSR id)
{
    auto keyPair = makeKeyPair();
    Instance temp = {id, keyPair.first, keyPair.second};
    followingMutex.lock();
    following.emplace(std::make_pair(temp.url, temp));
    followingMutex.unlock();
}

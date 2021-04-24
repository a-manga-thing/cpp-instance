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

const Instance& Globals::getFollowing(CSR id)
{
    followingMutex.lock_shared();
    const auto& ret = following[id];
    followingMutex.unlock_shared();
    return ret;
}

const Instance& Globals::addFollower(CSR publicKey, CSR id)
{
    Instance temp = {id, "", publicKey, ""};
    followersMutex.lock();
    followers.emplace(std::make_pair(temp.url, temp));
    followersMutex.unlock();
}

const Instance& Globals::addFollowing(CSR id)
{
    auto tuple = makeKeyPair(id);
    Instance temp = {id, std::get<2>(tuple), std::get<1>(tuple), std::get<0>(tuple)};
    followingMutex.lock();
    following.emplace(std::make_pair(temp.url, temp));
    followingMutex.unlock();
}

void Globals::removeFollower(CSR key)
{
    followersMutex.lock();
    if (followers.contains(key))
        followers.erase(key);
    followersMutex.unlock();
}

void Globals::forAllFollowers(std::function<void(const Instance&)> f)
{
    tryFollowMutex.lock_shared();
    for (auto& itr : followers) f(itr.second);
    tryFollowMutex.unlock_shared();
}

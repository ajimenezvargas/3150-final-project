#pragma once
#include "AS.h"
#include <algorithm>

using std::find;

AS::AS(uint32_t asn) : asn_(asn), propagation_rank_(-1){}
void AS::addProvider(AS* provider){
    if (provider && find(providers_.begin(), providers_.end(), provider) == providers_.end()){
        providers_.push_back(provider);
    }
}

void AS::addCustomer(AS* customer){
    if(customer && find(customers_.begin(), customers_.end(), customer) == customers_.end()) {
        customers_.push_back(customer);
    }
}

void AS::addPeer(AS* peer){
    if (peer && find(peers_.begin(), peers_.end(), peer) == peers_.end()){
        peers_.push_back(peer);
    }
}
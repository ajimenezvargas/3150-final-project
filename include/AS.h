#pragma once
#include <cstdint>
#include <vector>
#include <memory>
using std::vector, std::unique_ptr;
x
//AS Class, node in internet grah
class AS{
private:
    uint32_t asn_;           //ASN (unique ID)
    vector<AS*> providers_;  
    vector<AS*> customers_;
    vector<AS*> peers_;
    int propagation_rank_;
    //unique_ptr<Policy> policy_;

public:
    explicit AS(uint32_t asn);
    
    //getters
    uint32_t getASN() const {return asn_;}
    const vector<AS*>& getProviders() const {return providers_;}
    const vector<AS*>& getCustomers() const {return customers_;}
    const vector<AS*>& getPeers() const {return peers_;}
    int getPropagationRank() const {return propagation_rank_;}

    //setters
    void addProvider(AS* provider);
    void addCustomer(AS* customer);
    void addPeer(AS* peer);

    // Helper methods 
    bool hasCustomers() const {return !customers_.empty();}
    bool hasProviders() const {return !providers_.empty();}
};

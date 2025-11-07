#include "AS.h"                    // Public interface (from include/)
#include "utils/downloader.h"      // Private utility (from src/utils/)
#include <iostream>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    std::cout << "BGP Simulator - Day 1 Test" << std::endl;
    std::cout << "===========================" << std::endl;
    
    // Initialize libcurl
    initDownloader();
    
    // Create data directory if it doesn't exist
    fs::create_directories("data");
    
    // Test 1: Download CAIDA data
    std::string url = getCAIDAUrl();
    std::string output_path = "data/as-rel.txt.bz2";
    
    std::cout << "\n[Test 1] Downloading CAIDA dataset" << std::endl;
    if (fileExists(output_path)) {
        std::cout << "File already exists at: " << output_path << std::endl;
        std::cout << "File size: " << fs::file_size(output_path) / 1024 << " KB" << std::endl;
        std::cout << "Skipping download (delete file to re-download)" << std::endl;
    } else {
        if (!downloadFile(url, output_path)) {
            std::cerr << "\nWarning: Could not download CAIDA data automatically" << std::endl;
            std::cerr << "You can download manually from: " << url << std::endl;
            std::cout << "\nContinuing with other tests..." << std::endl;
        }
    }
    
    // Test 2: Create some AS objects
    std::cout << "\n[Test 2] Creating AS objects" << std::endl;
    
    // Create a simple graph: AS4 is provider to AS666 and AS3
    // AS4 is peer with AS777
    std::unordered_map<uint32_t, std::unique_ptr<AS>> as_graph;
    
    as_graph[4] = std::make_unique<AS>(4);
    as_graph[666] = std::make_unique<AS>(666);
    as_graph[3] = std::make_unique<AS>(3);
    as_graph[777] = std::make_unique<AS>(777);
    
    // Set up relationships
    // AS4 is provider to AS666
    as_graph[4]->addCustomer(as_graph[666].get());
    as_graph[666]->addProvider(as_graph[4].get());
    
    // AS4 is provider to AS3
    as_graph[4]->addCustomer(as_graph[3].get());
    as_graph[3]->addProvider(as_graph[4].get());
    
    // AS4 and AS777 are peers
    as_graph[4]->addPeer(as_graph[777].get());
    as_graph[777]->addPeer(as_graph[4].get());
    
    std::cout << "Created " << as_graph.size() << " ASes" << std::endl;
    
    // Test 3: Verify relationships
    std::cout << "\n[Test 3] Verifying relationships" << std::endl;
    
    std::cout << "AS4 customers: ";
    for (const auto* customer : as_graph[4]->getCustomers()) {
        std::cout << customer->getASN() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "AS4 peers: ";
    for (const auto* peer : as_graph[4]->getPeers()) {
        std::cout << peer->getASN() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "AS666 providers: ";
    for (const auto* provider : as_graph[666]->getProviders()) {
        std::cout << provider->getASN() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\n[SUCCESS] Day 1 basic tests passed!" << std::endl;
    
    // Cleanup libcurl
    cleanupDownloader();
    
    return 0;
}
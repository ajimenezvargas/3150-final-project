#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <cstdlib>

// Check if libcurl headers are available
#ifdef __has_include
#  if __has_include(<curl/curl.h>)
#    define HAS_LIBCURL 1
#    include <curl/curl.h>
#  else
#    define HAS_LIBCURL 0
#  endif
#else
#  define HAS_LIBCURL 0
#endif

namespace fs = std::filesystem;

#if HAS_LIBCURL

/**
 * Callback function for writing downloaded data to file
 */
static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    file->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}

/**
 * Callback function for progress reporting
 */
static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                           curl_off_t ultotal, curl_off_t ulnow) {
    (void)clientp;  // Unused
    (void)ultotal;  // Unused
    (void)ulnow;    // Unused
    
    if (dltotal > 0) {
        int percentage = static_cast<int>((dlnow * 100) / dltotal);
        std::cout << "\rDownloading: " << percentage << "% "
                  << "(" << dlnow / 1024 << " KB / " << dltotal / 1024 << " KB)" 
                  << std::flush;
    }
    return 0;
}

/**
 * Downloads a file using libcurl
 */
inline bool downloadFileLibcurl(const std::string& url, const std::string& output_path) {
    CURL* curl;
    CURLcode res;
    
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize libcurl" << std::endl;
        return false;
    }
    
    std::ofstream outfile(output_path, std::ios::binary);
    if (!outfile) {
        std::cerr << "Failed to open file for writing: " << output_path << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    
    std::cout << "Downloading from: " << url << std::endl;
    std::cout << "Using libcurl..." << std::endl;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outfile);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    
    res = curl_easy_perform(curl);
    
    outfile.close();
    
    if (res != CURLE_OK) {
        std::cerr << "\nDownload failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        if (fs::exists(output_path)) {
            fs::remove(output_path);
        }
        return false;
    }
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    
    std::cout << "\nDownload complete! HTTP " << response_code << std::endl;
    std::cout << "Saved to: " << output_path << std::endl;
    
    if (!fs::exists(output_path) || fs::file_size(output_path) == 0) {
        std::cerr << "Error: Downloaded file is empty or missing" << std::endl;
        return false;
    }
    
    std::cout << "File size: " << fs::file_size(output_path) / 1024 << " KB" << std::endl;
    return true;
}

#endif // HAS_LIBCURL

/**
 * Downloads a file using system curl command (fallback)
 */
inline bool downloadFileCurlCommand(const std::string& url, const std::string& output_path) {
    std::cout << "Downloading from: " << url << std::endl;
    std::cout << "Using system curl command..." << std::endl;
    
    std::string command = "curl -L -o \"" + output_path + "\" \"" + url + "\" 2>&1";
    
    int result = std::system(command.c_str());
    
    if (result != 0) {
        std::cerr << "\nDownload failed with exit code: " << result << std::endl;
        if (fs::exists(output_path)) {
            fs::remove(output_path);
        }
        return false;
    }
    
    if (!fs::exists(output_path) || fs::file_size(output_path) == 0) {
        std::cerr << "Error: Downloaded file is empty or missing" << std::endl;
        return false;
    }
    
    std::cout << "Download complete!" << std::endl;
    std::cout << "Saved to: " << output_path << std::endl;
    std::cout << "File size: " << fs::file_size(output_path) / 1024 << " KB" << std::endl;
    
    return true;
}

/**
 * Downloads a file from a URL to a local path
 * Automatically chooses libcurl if available, otherwise uses system curl
 */
inline bool downloadFile(const std::string& url, const std::string& output_path) {
#if HAS_LIBCURL
    return downloadFileLibcurl(url, output_path);
#else
    return downloadFileCurlCommand(url, output_path);
#endif
}

/**
 * Gets the CAIDA dataset URL for the previous month
 * Format: https://publicdata.caida.org/datasets/as-relationships/serial-1/YYYYMM01.as-rel.txt.bz2
 */
inline std::string getCAIDAUrl() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    tm.tm_mon -= 1;
    if (tm.tm_mon < 0) {
        tm.tm_mon = 11;
        tm.tm_year -= 1;
    }
    
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m", &tm);
    
    std::string url = "https://publicdata.caida.org/datasets/as-relationships/serial-1/";
    url += buffer;
    url += "01.as-rel.txt.bz2";
    
    return url;
}

/**
 * Checks if file already exists to avoid re-downloading
 */
inline bool fileExists(const std::string& path) {
    return fs::exists(path) && fs::file_size(path) > 0;
}

/**
 * Initialize downloader (call once at program start)
 */
inline void initDownloader() {
#if HAS_LIBCURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::cout << "Downloader: Using libcurl" << std::endl;
#else
    std::cout << "Downloader: Using system curl command" << std::endl;
#endif
}

/**
 * Cleanup downloader (call once at program end)
 */
inline void cleanupDownloader() {
#if HAS_LIBCURL
    curl_global_cleanup();
#endif
}
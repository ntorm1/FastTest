#include "exchange/asset.hpp"
#include "exchange/exchange.hpp"
#include "exchange/exchange_impl.hpp"


BEGIN_FASTTEST_NAMESPACE

//============================================================================
FastTestResult<bool> Exchange::init() noexcept {
  auto path = std::filesystem::path(m_source);
  if (!std::filesystem::exists(path)) {
    return Err("File does not exist: ");
  }
  if (std::filesystem::is_directory(path)) {
    return initDir();
  }
  return Err("File is not a directory: ");
}

//============================================================================
FastTestResult<bool> Exchange::initDir() noexcept {
  // get all the files in the directory
  std::filesystem::path path(m_source);
  std::vector<std::filesystem::path> files;
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    if (entry.is_regular_file()) {
      files.push_back(entry.path());
    }
    if (entry.is_directory()) {
      return Err("Exchange source directory contains subdirectories: {}",
                 entry.path().string());
    }
    if (entry.path().extension() != ".csv") {
      return Err("Exchange source directory contains non-csv file: {}"
                 ", found extension: {}, expected: .csv",
                 entry.path().string(), path.extension().string());
    }
    String asset_id = entry.path().stem().string();
    auto asset = Asset(asset_id, m_impl->assets.size());
    asset.setSource(entry.path().string());
    m_impl->assets.push_back(std::move(asset));
  }
  if (files.empty()) {
    return Err("Exchange source: {} directory is empty", m_source);
  }

  // make sure have datetime format
  if (!m_impl->datetime_format) {
    return Err("{}", "Datetime format is required for loading CSV files");
  }

  // load the files
  std::vector<std::thread> threads;
  String msg = "";
  std::mutex m_mutex;
  for (auto &asset : m_impl->assets) {
    threads.push_back(std::thread([this, &asset, &m_mutex, &msg]() {
      auto res = asset.loadCSV(*(this->m_impl->datetime_format));
      if (!res) {
        std::lock_guard<std::mutex> lock(m_mutex);
        String error = res.error().what();
        String error_msg =
            std::format("Error loading asset: {} - {}\n", asset.id, error);
        msg += error_msg;
      }
    }));
  }
  // Join all threads
  for (auto &thread : threads) {
    thread.join();
  }
  if (!msg.empty()) {
    return Err("Error loading exchange: {}", msg);
  }
  return true;
}

//============================================================================
FastTestResult<bool> initDir() noexcept { return true; }

END_FASTTEST_NAMESPACE
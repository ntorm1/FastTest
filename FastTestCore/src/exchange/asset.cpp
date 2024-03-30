#include "exchange/asset.hpp"

#include "ft_time.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
FastTestResult<bool> Asset::loadCSV(String const &datetime_format) {
  assert(source);
  try {
    // Open the file
    std::ifstream file(*source);
    if (!file.is_open()) {
      return Err("Failed to open file: {}", *source);
    }
    // get row count
    rows = 0;
    std::string line;
    while (std::getline(file, line)) {
      rows++;
    }
    rows--;
    file.clear();                 // Clear any error flags
    file.seekg(0, std::ios::beg); // Move the file pointer back to the start

    // Parse headers
    if (std::getline(file, line)) {
      std::stringstream ss(line);
      std::string columnName;
      int columnIndex = 0;

      // Skip the first column (date)
      std::getline(ss, columnName, ',');
      while (std::getline(ss, columnName, ',')) {
        headers[columnName] = columnIndex;
        columnIndex++;
      }
    } else {
      return Err("{}", "Could not parse headers");
    }
    cols = headers.size();
    resize(rows, cols);

    size_t row_counter = 0;
    while (std::getline(file, line)) {
      std::stringstream ss(line);

      // First column is datetime
      std::string timestamp, columnValue;
      std::getline(ss, timestamp, ',');

      // try to convert string to epoch time
      int64_t epoch_time = 0;
      if (datetime_format != "") {
        auto res = Time::strToEpoch(timestamp, datetime_format);
        if (res && res.value() > 0) {
          epoch_time = res.value();
        }
      } else {
        try {
          epoch_time = std::stoll(timestamp);
        } catch (...) {
        }
      }
      if (epoch_time == 0) {
        return Err("Invalid timestamp: {}, epoch time is: {}", timestamp, std::to_string(epoch_time));
      }
      timestamps[row_counter] = epoch_time;

      int col_idx = 0;
      while (std::getline(ss, columnValue, ',')) {
        double value = std::stod(columnValue);
        size_t index = row_counter * cols + col_idx;
        data[index] = value;
        col_idx++;
      }
      row_counter++;
    }
    return true;
  } catch (const std::exception &e) {
    return Err("Error loading CSV: {}", std::string(e.what()));
  } catch (...) {
    return Err("Error loading CSV {}: Unknown error", *source);
  }
}


END_FASTTEST_NAMESPACE
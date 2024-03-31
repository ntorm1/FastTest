#pragma once
#include "ft_macros.hpp"
#include "pch.hpp"

BEGIN_FASTTEST_NAMESPACE

class FastTestException : public std::exception {
private:
  std::string m_message;

public:
  FastTestException(std::string message) noexcept
      : m_message(std::move(message)) {}
  ~FastTestException() noexcept override = default;

  const char *what() const noexcept override { return m_message.c_str(); }
};


END_FASTTEST_NAMESPACE
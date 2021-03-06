#define ASIO_STANDALONE
#include <asio.hpp> // IWYU pragma: keep
// IWYU pragma: no_include <asio/executor_work_guard.hpp>
// IWYU pragma: no_include <asio/impl/io_context.hpp>
// IWYU pragma: no_include <asio/impl/io_context.ipp>
// IWYU pragma: no_include <asio/io_context.hpp>

#include <csignal>                       // for signal, SIGINT
#include <iostream>                      // for operator<<, endl, basic_ostream
#include <memory>                        // for allocator
#include "config.hpp"                    // for Config
#include "mappings.hpp"                  // for Mappings

asio::io_context io_context;

extern "C" void stop(int sig) {
  (void)sig;
  io_context.stop();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Missing command line argument");
  }

  Config config = Config::parse(io_context, argv[1]);

  Mappings mappings(config);

  std::signal(SIGINT, &stop);

  asio::executor_work_guard<asio::io_context::executor_type> work
    = asio::make_work_guard(io_context);
  io_context.run();
}


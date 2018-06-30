#pragma once

namespace pbcache {

enum class State {
  OK,
  EMPTY,
  NO_BACKEND,
  FOUND_NO_MESSAGE_TYPE,
  FOUND_NO_MESSAGE_PROTOTYPE,
  PARSE_ERROR,
  DATA_ERROR,
};

}
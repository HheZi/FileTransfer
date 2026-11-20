#ifndef CLIENT_OFFER_RESULT_HPP
#define CLIENT_OFFER_RESULT_HPP

#include "FileOffer.pb.h"

struct OfferResult {
  bool file_accepted;

  OfferResult() = default;
  OfferResult(const FileAnswer &answer) : file_accepted(answer.accepted()) {}
};

#endif

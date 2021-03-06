// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

namespace cryptonote {
class ITxPoolObserver {
public:
  virtual ~ITxPoolObserver() {
  }

  virtual void txDeletedFromPool() = 0;
};
}

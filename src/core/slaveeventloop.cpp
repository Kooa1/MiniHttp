//
// Created by 66 on 2026/6/5.
//

#include "slaveeventloop.h"

void SlaveEventLoop::closeAllConnection() {
    auto tmp = connections_;
    for (auto &conn: tmp) {
        conn->mClose();
    }
}

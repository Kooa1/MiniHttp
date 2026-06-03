//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_CHANNEL_H
#define MINIHTTP_CHANNEL_H

#include <cstdint>
#include <functional>


class Channel {
public:
    using EventCallback = std::function<void()>;

    Channel(int fd, uint32_t events);

    ~Channel() = default;

    int fd() const { return fd_; };

    uint32_t events() const { return events_; };

    void setReadCallBack(EventCallback cb) { read_cb_ = cb; };

    void handleEvent() const;

private:
    int fd_;
    uint32_t events_;
    EventCallback read_cb_;
};


#endif //MINIHTTP_CHANNEL_H

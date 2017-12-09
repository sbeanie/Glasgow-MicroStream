#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

template <typename T>
class Subscriber {

public:
    virtual void receive(T) {};
};
#endif

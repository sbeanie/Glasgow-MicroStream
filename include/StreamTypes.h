#ifndef _STREAM_TYPES_H_
#define _STREAM_TYPES_H_

template <typename T>
class Sink;

template <typename INPUT, typename OUTPUT>
class TwoTypeStream;

template <typename T>
class Stream: public TwoTypeStream<T, T> {};

template <typename T>
class FilterStream;

template <typename T>
class Window;

template <typename INPUT, typename OUTPUT>
class MapStream;

template <typename T>
class SplitStream;

#endif
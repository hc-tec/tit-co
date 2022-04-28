//
// Created by titto on 2022/4/28.
//

#ifndef TIT_COROUTINE_TRAITS_H
#define TIT_COROUTINE_TRAITS_H

template <char const* v>
struct Str2Type {
  enum { value = v };
};

template <int T>
struct Int2Type {
  enum { value = T };
};

#endif  // TIT_COROUTINE_TRAITS_H

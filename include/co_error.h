//
// Created by titto on 2022/4/13.
//

#ifndef TIT_COROUTINE_CO_ERROR_H
#define TIT_COROUTINE_CO_ERROR_H

#include <cerrno>

namespace tit {

namespace co {

const char* stderror() {
  if (errno == EAGAIN) {
    return "EAGAIN";
  } else if (errno == EWOULDBLOCK) {
    return "EWOULDBLOCK";
  } else if (errno == EBADF) {
    return "EBADF";
  } else if (errno == EDESTADDRREQ) {
    return "EDESTADDRREQ";
  } else if (errno == EDQUOT) {
    return "EDQUOT";
  } else if (errno == EFAULT) {
    return "EFAULT";
  } else if (errno == EFBIG) {
    return "EFBIG";
  } else if (errno == EINTR) {
    return "EINTR";
  } else if (errno == EINVAL) {
    return "EINVAL";
  } else if (errno == EIO) {
    return "EIO";
  } else if (errno == ENOSPC) {
    return "ENOSPC";
  } else if (errno == EPERM) {
    return "EPERM";
  } else if (errno == EPIPE) {
    return "EPIPE";
  }
  return "unknown error";
}

}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_CO_ERROR_H

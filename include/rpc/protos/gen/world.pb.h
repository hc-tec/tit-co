// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: world.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_world_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_world_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3020000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3020000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_world_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_world_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_world_2eproto;
class WorldProtocolBuf;
struct WorldProtocolBufDefaultTypeInternal;
extern WorldProtocolBufDefaultTypeInternal _WorldProtocolBuf_default_instance_;
PROTOBUF_NAMESPACE_OPEN
template<> ::WorldProtocolBuf* Arena::CreateMaybeMessage<::WorldProtocolBuf>(Arena*);
PROTOBUF_NAMESPACE_CLOSE

// ===================================================================

class WorldProtocolBuf final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:WorldProtocolBuf) */ {
 public:
  inline WorldProtocolBuf() : WorldProtocolBuf(nullptr) {}
  ~WorldProtocolBuf() override;
  explicit PROTOBUF_CONSTEXPR WorldProtocolBuf(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  WorldProtocolBuf(const WorldProtocolBuf& from);
  WorldProtocolBuf(WorldProtocolBuf&& from) noexcept
    : WorldProtocolBuf() {
    *this = ::std::move(from);
  }

  inline WorldProtocolBuf& operator=(const WorldProtocolBuf& from) {
    CopyFrom(from);
    return *this;
  }
  inline WorldProtocolBuf& operator=(WorldProtocolBuf&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const WorldProtocolBuf& default_instance() {
    return *internal_default_instance();
  }
  static inline const WorldProtocolBuf* internal_default_instance() {
    return reinterpret_cast<const WorldProtocolBuf*>(
               &_WorldProtocolBuf_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(WorldProtocolBuf& a, WorldProtocolBuf& b) {
    a.Swap(&b);
  }
  inline void Swap(WorldProtocolBuf* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(WorldProtocolBuf* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  WorldProtocolBuf* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<WorldProtocolBuf>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const WorldProtocolBuf& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const WorldProtocolBuf& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to, const ::PROTOBUF_NAMESPACE_ID::Message& from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(WorldProtocolBuf* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "WorldProtocolBuf";
  }
  protected:
  explicit WorldProtocolBuf(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kResultFieldNumber = 1,
  };
  // int32 result = 1;
  void clear_result();
  int32_t result() const;
  void set_result(int32_t value);
  private:
  int32_t _internal_result() const;
  void _internal_set_result(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:WorldProtocolBuf)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    int32_t result_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_world_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// WorldProtocolBuf

// int32 result = 1;
inline void WorldProtocolBuf::clear_result() {
  _impl_.result_ = 0;
}
inline int32_t WorldProtocolBuf::_internal_result() const {
  return _impl_.result_;
}
inline int32_t WorldProtocolBuf::result() const {
  // @@protoc_insertion_point(field_get:WorldProtocolBuf.result)
  return _internal_result();
}
inline void WorldProtocolBuf::_internal_set_result(int32_t value) {
  
  _impl_.result_ = value;
}
inline void WorldProtocolBuf::set_result(int32_t value) {
  _internal_set_result(value);
  // @@protoc_insertion_point(field_set:WorldProtocolBuf.result)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_world_2eproto

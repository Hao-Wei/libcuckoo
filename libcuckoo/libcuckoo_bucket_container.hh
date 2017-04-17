#ifndef LIBCUCKOO_BUCKET_CONTAINER_H
#define LIBCUCKOO_BUCKET_CONTAINER_H

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

/**
 * libcuckoo_bucket_container manages storage of key-value pairs for the table.
 * It stores the items inline in uninitialized memory, and keeps track of which
 * slots have live data and which do not. It also stores a partial hash for
 * each live key. It is sized by powers of two.
 *
 * @tparam Key type of keys in the table
 * @tparam T type of values in the table
 * @tparam Alloc type of key-value pair allocator
 * @tparam Partial type of partial keys
 * @tparam SLOT_PER_BUCKET number of slots for each bucket in the table
 */
template <class Key, class T, class Alloc, class Partial,
          std::size_t SLOT_PER_BUCKET>
class libcuckoo_bucket_container {
public:
  using key_type = Key;
  using mapped_type = T;
  struct value_type {
    const key_type &first;
    mapped_type &second;
  };
  using allocator_type = Alloc;
  using partial_t = Partial;
  using size_type = std::size_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;

  /*
   * The bucket type holds SLOT_PER_BUCKET key-value pairs, along with their
   * partial keys and occupancy info. It uses aligned_storage arrays to store
   * the keys and values to allow constructing and destroying key-value pairs
   * in place. The lifetime of bucket data should be managed by the container.
   * It is the user's responsibility to confirm whether the data they are
   * accessing is live or not.
   */
  class bucket {
  public:
    bucket() noexcept : occupied_({}) {}

    const key_type &key(size_type ind) const {
      return *static_cast<const key_type *>(
          static_cast<const void *>(&keys_[ind]));
    }
    key_type &&movable_key(size_type ind) {
      return std::move(
          *static_cast<key_type *>(static_cast<void *>(&keys_[ind])));
    }

    const mapped_type &mapped(size_type ind) const {
      return *static_cast<const mapped_type *>(
          static_cast<const void *>(&mappeds_[ind]));
    }
    mapped_type &mapped(size_type ind) {
      return *static_cast<mapped_type *>(static_cast<void *>(&mappeds_[ind]));
    }

    partial_t partial(size_type ind) const { return partials_[ind]; }
    partial_t &partial(size_type ind) { return partials_[ind]; }

    bool occupied(size_type ind) const { return occupied_[ind]; }
    bool &occupied(size_type ind) { return occupied_[ind]; }

  private:
    std::array<typename std::aligned_storage<sizeof(key_type),
                                             alignof(key_type)>::type,
               SLOT_PER_BUCKET>
        keys_;
    std::array<typename std::aligned_storage<sizeof(mapped_type),
                                             alignof(mapped_type)>::type,
               SLOT_PER_BUCKET>
        mappeds_;
    std::array<partial_t, SLOT_PER_BUCKET> partials_;
    std::array<bool, SLOT_PER_BUCKET> occupied_;
  };

private:
  using alloc_traits_ = std::allocator_traits<allocator_type>;
  using key_traits_ = typename alloc_traits_::template rebind_traits<key_type>;
  using mapped_traits_ =
      typename alloc_traits_::template rebind_traits<mapped_type>;
  using bucket_traits_ = typename alloc_traits_::template rebind_traits<bucket>;

public:
  libcuckoo_bucket_container(size_type hp, const allocator_type &allocator)
      : allocator_(allocator), key_allocator_(allocator),
        mapped_allocator_(allocator), bucket_allocator_(allocator),
        hashpower_(hp),
        buckets_(bucket_traits_::allocate(bucket_allocator_, size())) {
    // The bucket default constructor is nothrow, so we don't have to
    // worry about dealing with exceptions when constructing all the
    // elements.
    static_assert(std::is_nothrow_constructible<bucket>::value,
                  "libcuckoo_bucket_container requires bucket to be nothrow "
                  "constructible");
    for (size_type i = 0; i < size(); ++i) {
      bucket_traits_::construct(bucket_allocator_, &buckets_[i]);
    }
  }

  ~libcuckoo_bucket_container() noexcept { destroy_buckets(); }

  libcuckoo_bucket_container(const libcuckoo_bucket_container &bc)
      : allocator_(alloc_traits_::select_on_container_copy_construction(
            bc.allocator_)),
        key_allocator_(key_traits_::select_on_container_copy_construction(
            bc.key_allocator_)),
        mapped_allocator_(mapped_traits_::select_on_container_copy_construction(
            bc.mapped_allocator_)),
        bucket_allocator_(bucket_traits_::select_on_container_copy_construction(
            bc.bucket_allocator_)),
        hashpower_(bc.hashpower()),
        buckets_(transfer(bc.hashpower(), bc, std::false_type())) {}

  libcuckoo_bucket_container(libcuckoo_bucket_container &&bc)
      : allocator_(std::move(bc.allocator_)),
        bucket_allocator_(std::move(bc.bucket_allocator_)),
        key_allocator_(std::move(bc.key_allocator_)),
        mapped_allocator_(std::move(bc.mapped_allocator_)),
        hashpower_(bc.hashpower()), buckets_(std::move(bc.buckets_)) {
    // De-activate the other buckets container
    bc.buckets_ = nullptr;
  }

  libcuckoo_bucket_container &operator=(const libcuckoo_bucket_container &bc) {
    destroy_buckets();
    copy_allocator(
        allocator_, bc.allocator_,
        typename alloc_traits_::propagate_on_container_copy_assignment());
    copy_allocator(
        key_allocator_, bc.key_allocator_,
        typename key_traits_::propagate_on_container_copy_assignment());
    copy_allocator(
        mapped_allocator_, bc.mapped_allocator_,
        typename mapped_traits_::propagate_on_container_copy_assignment());
    copy_allocator(
        bucket_allocator_, bc.bucket_allocator_,
        typename bucket_traits_::propagate_on_container_copy_assignment());
    hashpower_ = bc.hashpower();
    buckets_ = transfer(hashpower(), bc, std::false_type());
    return *this;
  }

  libcuckoo_bucket_container &operator=(libcuckoo_bucket_container &&bc) {
    destroy_buckets();
    move_allocator(
        allocator_, bc.allocator_,
        typename alloc_traits_::propagate_on_container_move_assignment());
    move_allocator(
        key_allocator_, bc.key_allocator_,
        typename key_traits_::propagate_on_container_move_assignment());
    move_allocator(
        mapped_allocator_, bc.mapped_allocator_,
        typename mapped_traits_::propagate_on_container_move_assignment());
    hashpower(bc.hashpower());
    // When considering whether or not to move the bucket memory, we only need
    // to look at bucket_allocator_, since it is the only allocator used to
    // actually allocate memory.
    move_assign(
        bc, typename bucket_traits_::propagate_on_container_move_assignment());
    return *this;
  }

  size_type hashpower() const {
    return hashpower_.load(std::memory_order_acquire);
  }

  void hashpower(size_type val) {
    hashpower_.store(val, std::memory_order_release);
  }

  size_type size() const { return 1UL << hashpower(); }

  allocator_type get_allocator() const { return allocator_; }

  bucket &operator[](size_type i) { return buckets_[i]; }
  const bucket &operator[](size_type i) const { return buckets_[i]; }

  // Constructs live data in a bucket
  template <typename K, typename... Args>
  void setKV(size_type ind, size_type slot, partial_t p, K &&k,
             Args &&... args) {
    bucket &b = buckets_[ind];
    assert(!b.occupied(slot));
    b.partial(slot) = p;
    b.occupied(slot) = true;
    key_traits_::construct(key_allocator_, std::addressof(b.key(slot)),
                           std::forward<K>(k));
    mapped_traits_::construct(mapped_allocator_, std::addressof(b.mapped(slot)),
                              std::forward<Args>(args)...);
  }

  // Destroys live data in a bucket
  void eraseKV(size_type ind, size_type slot) {
    bucket &b = buckets_[ind];
    assert(b.occupied(slot));
    b.occupied(slot) = false;
    key_traits_::destroy(key_allocator_, std::addressof(b.key(slot)));
    mapped_traits_::destroy(mapped_allocator_, std::addressof(b.mapped(slot)));
  }

  // Moves data between two buckets in the container
  void moveKV(size_type dst_ind, size_type dst_slot, size_type src_ind,
              size_type src_slot) {
    bucket &dst = buckets_[dst_ind];
    bucket &src = buckets_[src_ind];
    assert(src.occupied(src_slot));
    assert(!dst.occupied(dst_slot));
    setKV(dst_ind, dst_slot, src.partial(src_slot), src.movable_key(src_slot),
          std::move(src.mapped(src_slot)));
    eraseKV(src_ind, src_slot);
  }

  // Destroys all the live data in the buckets
  void clear() noexcept {
    static_assert(
        std::is_nothrow_destructible<key_type>::value &&
            std::is_nothrow_destructible<mapped_type>::value,
        "libcuckoo_bucket_container requires key and value to be nothrow "
        "destructible");
    for (size_type i = 0; i < size(); ++i) {
      bucket &b = buckets_[i];
      for (size_type j = 0; j < SLOT_PER_BUCKET; ++j) {
        if (b.occupied(j)) {
          eraseKV(i, j);
        }
      }
    }
  }

  // Creates a new container of hashpower `new_hp`, transfers the old data into
  // it, and destroys the old container. Will not shrink the container.
  void resize(size_type new_hp) {
    assert(new_hp >= hashpower());
    typename bucket_traits_::pointer new_buckets =
        transfer(new_hp, *this, std::true_type());
    destroy_buckets();
    buckets_ = new_buckets;
    hashpower(new_hp);
    hashpower_ = new_hp;
  }

private:
  // true here means the allocators from `src` are propagated on copy
  template <typename A>
  void copy_allocator(A &dst, const A &src, std::true_type) {
    dst = src;
  }

  template <typename A>
  void copy_allocator(A &dst, const A &src, std::false_type) {}

  // true here means the allocators from `src` are propagated on move
  template <typename A> void move_allocator(A &dst, A &src, std::true_type) {
    dst = std::move(src);
  }

  template <typename A> void move_allocator(A &dst, A &src, std::false_type) {}

  // true here means the bucket allocator should be propagated
  void move_assign(libcuckoo_bucket_container &src, std::true_type) {
    bucket_allocator_ = std::move(src.bucket_allocator_);
    buckets_ = src.buckets_;
    src.buckets_ = nullptr;
  }

  void move_assign(libcuckoo_bucket_container &src, std::false_type) {
    if (bucket_allocator_ == src.bucket_allocator_) {
      buckets_ = src.buckets_;
      src.buckets_ = nullptr;
    } else {
      buckets_ = transfer(hashpower(), src, std::true_type());
    }
  }

  void destroy_buckets() noexcept {
    if (buckets_ == nullptr) {
      return;
    }
    // The bucket default constructor is nothrow, so we don't have to
    // worry about dealing with exceptions when constructing all the
    // elements.
    static_assert(std::is_nothrow_destructible<bucket>::value,
                  "libcuckoo_bucket_container requires bucket to be nothrow "
                  "destructible");
    clear();
    for (size_type i = 0; i < size(); ++i) {
      bucket_traits_::destroy(bucket_allocator_, &buckets_[i]);
    }
    bucket_traits_::deallocate(bucket_allocator_, buckets_, size());
  }

  // `true` here refers to whether or not we should move
  void move_or_copy(size_type dst_ind, size_type dst_slot, bucket &src,
                    size_type src_slot, std::true_type) {
    setKV(dst_ind, dst_slot, src.partial(src_slot), src.movable_key(src_slot),
          std::move(src.mapped(src_slot)));
  }

  void move_or_copy(size_type dst_ind, size_type dst_slot, bucket &src,
                    size_type src_slot, std::false_type) {
    setKV(dst_ind, dst_slot, src.partial(src_slot), src.key(src_slot),
          src.val(src_slot));
  }

  template <bool B>
  typename bucket_traits_::pointer transfer(
      size_type dst_hp,
      typename std::conditional<B, libcuckoo_bucket_container &,
                                const libcuckoo_bucket_container &>::type src,
      std::integral_constant<bool, B> move) {
    assert(dst_hp >= src.hashpower());
    libcuckoo_bucket_container dst(dst_hp, get_allocator());
    // Move/copy all occupied slots of the source buckets
    for (size_t i = 0; i < src.size(); ++i) {
      for (size_t j = 0; j < SLOT_PER_BUCKET; ++j) {
        if (src.buckets_[i].occupied(j)) {
          dst.move_or_copy(i, j, src.buckets_[i], j, move);
        }
      }
    }
    // Take away the pointer from `dst` and return it
    typename bucket_traits_::pointer dst_pointer = dst.buckets_;
    dst.buckets_ = nullptr;
    return dst_pointer;
  }

  typename alloc_traits_::allocator_type allocator_;
  typename key_traits_::allocator_type key_allocator_;
  typename mapped_traits_::allocator_type mapped_allocator_;
  typename bucket_traits_::allocator_type bucket_allocator_;
  // This needs to be atomic, since it can be read and written by multiple
  // threads not necessarily synchronized by a lock.
  std::atomic<size_type> hashpower_;
  // These buckets are protected by striped locks (external to the
  // BucketContainer), which must be obtained before accessing a bucket.
  typename bucket_traits_::pointer buckets_;
};

#endif // LIBCUCKOO_BUCKET_CONTAINER_H

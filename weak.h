#pragma once

#include "sw_fwd.h"  // Forward declaration

template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() : block_(nullptr), ptr_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->weak_cnt += 1;
        }
    }
    WeakPtr(WeakPtr&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    template <typename S>
    WeakPtr(const WeakPtr<S>& other) {
        block_ = other.GetBlock();
        ptr_ = other.Get();
        if (block_) {
            block_->weak_cnt += 1;
        }
    }
    template <typename S>
    WeakPtr(WeakPtr<S>&& other) {
        block_ = other.GetBlock();
        ptr_ = other.Get();
        if (block_) {
            block_->weak_cnt += 1;
        }
        other.Reset();
    }

    // Demote `SharedPtr`
    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.GetBlock();
        ptr_ = other.Get();
        if (block_) {
            block_->weak_cnt += 1;
        }
    }
    template <typename S>
    WeakPtr(const SharedPtr<S>& other) {
        block_ = other.GetBlock();
        ptr_ = other.Get();
        if (block_) {
            block_->weak_cnt += 1;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        auto tmp = block_;
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->weak_cnt += 1;
        }
        if (tmp) {
            tmp->weak_cnt -= 1;
            if (tmp->strong_cnt == 0 && tmp->weak_cnt == 0) {
                delete tmp;
            }
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (this == &other) {
            return *this;
        }
        auto tmp = block_;
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        if (tmp) {
            tmp->weak_cnt -= 1;
            if (tmp->strong_cnt == 0 && tmp->weak_cnt == 0) {
                delete tmp;
            }
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_) {
            block_->weak_cnt -= 1;
            if (block_->strong_cnt == 0 && block_->weak_cnt == 0) {
                delete block_;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        auto old_block = block_;
        block_ = nullptr;
        ptr_ = nullptr;
        if (old_block) {
            old_block->weak_cnt -= 1;
            if (old_block->strong_cnt == 0 && old_block->weak_cnt == 0) {
                delete old_block;
            }
        }
    }
    void Swap(WeakPtr& other) {
        if (this == &other) {
            return;
        }
        auto tmp = block_;
        auto tmp_ptr = ptr_;
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = tmp;
        other.ptr_ = tmp_ptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->strong_cnt;
        }
        return 0;
    }
    bool Expired() const {
        if (!block_) {
            return true;
        }
        if (block_->strong_cnt == 0) {
            return true;
        }
        return false;
    }
    SharedPtr<T> Lock() const {
        if (!Expired()) {
            return SharedPtr(*this);
        }
        return SharedPtr<T>();
    }

    ControlBlockBase* GetBlock() const {
        return block_;
    }
    T* Get() const {
        return ptr_;
    }

private:
    ControlBlockBase* block_;
    T* ptr_;
};

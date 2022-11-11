#pragma once

#include <type_traits>
#include <stddef.h>
#include <utility>

template <typename T, size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    CompressedPairElement() : value(T()) {
    }
    CompressedPairElement(T& value) : value(value) {
    }
    CompressedPairElement(T&& value) : value(std::forward<T>(value)) {
    }

    T& GetElement() {
        return value;
    }
    const T& GetElement() const {
        return value;
    }

    T value;
};

template <typename T, size_t I>
struct CompressedPairElement<T, I, true> : public T {
    CompressedPairElement() : T(T()) {
    }
    CompressedPairElement(T& value) : T(value) {
    }
    CompressedPairElement(T&& value) : T(std::forward<T>(value)) {
    }

    T& GetElement() {
        return *this;
    }
    const T& GetElement() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : CompressedPairElement<F, 0>, CompressedPairElement<S, 1> {
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

public:
    CompressedPair() : First(F()), Second(S()) {
    }
    CompressedPair(F& first, S& second) : First(first), Second(second) {
    }
    CompressedPair(F&& first, S& second) : First(std::forward<F>(first)), Second(second) {
    }
    CompressedPair(F& first, S&& second) : First(first), Second(std::forward<S>(second)) {
    }
    CompressedPair(F&& first, S&& second)
        : First(std::forward<F>(first)), Second(std::forward<S>(second)) {
    }

    F& GetFirst() {
        return First::GetElement();
    }
    const F& GetFirst() const {
        return First::GetElement();
    }

    S& GetSecond() {
        return Second::GetElement();
    };
    const S& GetSecond() const {
        return Second::GetElement();
    };
};

#pragma once
// C++20 coroutine support for IEnumerator-style state machines
#include <coroutine>
#include <iterator>
#include <exception>

namespace AnodyneSharp {

// Simple C++20 coroutine that replaces C# IEnumerator
// Usage: co_yield nullptr; (replaces yield return null;)
//        co_return; (replaces yield break;)
struct Coroutine {
    struct promise_type {
        void* current_value = nullptr;

        Coroutine get_return_object() {
            return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(std::nullptr_t) noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    Coroutine() = default;
    explicit Coroutine(handle_type h) : handle(h) {}
    Coroutine(Coroutine&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
    Coroutine& operator=(Coroutine&& other) noexcept {
        if (handle) handle.destroy();
        handle = other.handle;
        other.handle = nullptr;
        return *this;
    }
    ~Coroutine() { if (handle) handle.destroy(); }

    // Disallow copy
    Coroutine(const Coroutine&) = delete;
    Coroutine& operator=(const Coroutine&) = delete;

    // Returns true if there is more to run, false when finished
    bool MoveNext() {
        if (!handle || handle.done()) return false;
        handle.resume();
        return !handle.done();
    }

    bool Done() const { return !handle || handle.done(); }
    explicit operator bool() const { return !Done(); }

    handle_type handle{};
};

// IEnumerator alias (mirrors C# IEnumerator)
using IEnumerator = Coroutine;

} // namespace AnodyneSharp

// Bring into global scope
using AnodyneSharp::Coroutine;
using AnodyneSharp::IEnumerator;

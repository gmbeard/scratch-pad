#include <iostream>

template<typename Parent>
struct Poll;

template<typename T>
struct PollCallable;

template<typename Parent>
struct Poll {
    auto operator()() {
        return (*static_cast<Parent*>(this)).poll();
    }
};

template<typename T>
struct PollCallable : Poll<PollCallable<T>> 
{
    using Result = decltype(std::declval<T>()());

    template<bool = true,
             typename _NotAlreadyAPoll = 
                std::enable_if_t<!std::is_convertible_v<T*, Poll<T>*>>>
    PollCallable(T&& inner) :
        inner_{std::forward<T>(inner)}
    { }

    auto poll() -> Result {
        return (inner_)();
    }

private:
    T inner_;
};

template<typename Callable>
auto make_poll(
    Callable&& callable,
    std::enable_if_t<!std::is_convertible_v<Callable*, 
                                            Poll<Callable>*>>* = nullptr)
    -> PollCallable<Callable> 
{
    return PollCallable<Callable>{std::forward<Callable>(callable)};
}

template<typename Callable>
auto make_poll(
    Callable&& callable,
    std::enable_if_t<std::is_convertible_v<Callable*, 
                                           Poll<Callable>*>>* = nullptr)
    -> Callable&&
{
    return std::forward<Callable>(callable);
}

int main(int, char const**) {
    size_t i = 42;
    auto f = make_poll([&i]() { return i; });

    auto v1 = make_poll(std::move(f));

    auto v = v1();
    std::cerr << "Result: " << v << "\n";
}

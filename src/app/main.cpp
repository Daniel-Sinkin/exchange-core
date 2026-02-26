#include "exchange/core/core_types.hpp"
#include "exchange/core/matching_engine.hpp"
#include "exchange/core/pch.hpp"

#include <array>
#include <latch>
#include <print>
#include <thread>

namespace ds_exch
{

struct Job
{
    const std::string name;
    std::string product{"not worked"};
    std::jthread action{};
};
}  // namespace ds_exch

int main()
{
    using namespace ds_exch;
    std::array jobs = {Job{"Annika"}, Job{"Buru"}, Job{"Chuck"}};
    std::latch work_done{jobs.size()};
    std::latch start_cleanup{1};

    auto work = [&](Job& my_job)
    {
        my_job.product = my_job.name + " worked";
        work_done.count_down();
        start_cleanup.wait();
        my_job.product = my_job.name + " cleaned";
    };

    std::println("Work is starting...");
    for (auto& job : jobs)
    {
        job.action = std::jthread{work, std::ref(job)};
    }

    work_done.wait();
    std::println("done");
    for (auto const& job : jobs)
    {
        std::println("{}", job.product);
    }

    std::println("Workers are cleaning up...");
    start_cleanup.count_down();
    for (auto& job : jobs)
    {
        job.action.join();
    }

    std::println("Done:");
    for (const auto& job : jobs)
    {
        std::println("{}", job.product);
    }
}

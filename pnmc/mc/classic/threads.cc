#include "mc/classic/threads.hh"

namespace pnmc { namespace mc { namespace classic {

/*------------------------------------------------------------------------------------------------*/

threads::threads( const conf::configuration& conf, statistics& stats, bool& stop
                , const sdd::manager<sdd_conf>& manager, util::timer& beginnning)
    : finished(false), clock(), sdd_sampling()
{
  if (conf.max_time > std::chrono::duration<double>(0))
  {
    clock = std::thread([&]
            {
              while (not finished)
              {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (beginnning.duration() >= conf.max_time)
                {
                  stop = true;
                  break;
                }
              }
            });
  }

  if (conf.stats_conf.count(shared::stats::nb_sdd))
  {
    stats.sdd_ut_size.emplace();
    sdd_sampling = std::thread([&]
                   {
                     const auto sample_time = std::chrono::milliseconds(500);
                     auto last = std::chrono::system_clock::now();
                     while (not finished)
                     {
                       std::this_thread::sleep_for(std::chrono::milliseconds(100));
                       auto now = std::chrono::system_clock::now();
                       if ((now - last) >= sample_time)
                       {
                         stats.sdd_ut_size->emplace_back(manager.sdd_stats().size);
                         last = now;
                       }
                     }
                   });
  }
}

/*------------------------------------------------------------------------------------------------*/

threads::~threads()
{
  finished = true;
  if (clock.joinable())
  {
    clock.join();
  }
  if (sdd_sampling.joinable())
  {
    sdd_sampling.join();
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::classic

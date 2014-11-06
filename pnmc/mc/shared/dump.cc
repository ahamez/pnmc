#include "mc/shared/dump.hh"

namespace pnmc { namespace mc { namespace shared {

/*------------------------------------------------------------------------------------------------*/

void
dump_results(const conf::configuration& conf, const results& res)
{
  if (conf.results_json_file)
  {
    boost::filesystem::ofstream file(*conf.results_json_file);
    if (file.is_open())
    {
      cereal::JSONOutputArchive archive(file);
      archive(cereal::make_nvp("pnmc", res));
    }
    else
    {
      std::cerr << "Can't export results to " << *conf.results_json_file << std::endl;
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

}}} // namespace pnmc::mc::shared

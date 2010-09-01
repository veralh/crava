#include<ostream>
#include<vector>
#include<string>

#include "src/tasklist.h"
#include "nrlib/iotools/logkit.hpp"
#include "lib/utils.h"

std::vector<std::string> TaskList::task_(0);

void TaskList::viewAllTasks(void)
{
  size_t i;
  size_t size = task_.size();

  if (size > 0)
  {
    Utils::writeHeader("Suggested tasks");
    LogKit::LogFormatted(LogKit::Low,"\n");
    for (i=0; i<size; i++)
      LogKit::LogFormatted(LogKit::Low,"%d: %s \n", i+1, task_[i].c_str());
  }
}

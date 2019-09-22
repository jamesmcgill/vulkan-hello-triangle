// always include fmt before Application.h (includes windows.h and so does glfw)
#include <fmt/format.h>
#include "Application.h"

//----------------------------------------------------------------------------------------
int
main()
{
  try
  {
    Application app;
    app.init();
    app.run();
  }
  catch (const std::exception& e)
  {
    fmt::print("{}\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------------------

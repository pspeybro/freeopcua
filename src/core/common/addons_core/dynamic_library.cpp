/// @author Alexander Rykovanov 2013
/// @email rykovanov.as@gmail.com
/// @brief Dynamic library class.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///

#include "dynamic_library.h"

#include <opc/common/addons_core/errors.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif

namespace
{

  void* LoadLibrary(const char* path)
  {
#ifndef _WIN32
    void* library = dlopen(path, RTLD_LAZY);
    if (!library)
    {
      std::string msg;
      if (const char* err = dlerror())
      {
        msg = err;
      }
      THROW_ERROR2(UnableToLoadDynamicLibrary, path, msg);
    }
#else
	  //not implemented
	  void* library = 0;
	  THROW_ERROR1(UnableToLoadDynamicLibrary, path);
#endif
    return library;
  }

}

namespace Common
{

  DynamicLibrary::DynamicLibrary(const std::string& libraryPath)
    : Path(libraryPath)
    , Library(0)
  {
  }

  DynamicLibrary::~DynamicLibrary()
  {
    if (Library)
    {
#ifndef _WIN32
      dlclose(Library);
#endif
    }
  }

  void* DynamicLibrary::FindSymbol(const std::string& funcName)
  {
#ifndef _WIN32
    if (!Library)
    {
      Library = LoadLibrary(Path.c_str());
    }

    void* func = dlsym(Library, funcName.c_str());
    if (!func)
    {
      std::string msg;
      if (const char* err =dlerror())
      {
        msg = err;
      }
      THROW_ERROR3(UnableToFundSymbolInTheLibrary, funcName, Path, msg);
    }
#else
	//not implemented
	  void* func = 0;
	  THROW_ERROR2(UnableToFundSymbolInTheLibrary, funcName, Path);
#endif
    return func;
  }

}

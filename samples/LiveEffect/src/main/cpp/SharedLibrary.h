#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include <dlfcn.h>
#include <logging_macros.h>
#include <cstring>
#include <string>

class SharedLibrary {
public:
    std::string so_file ;
    void * dl_handle = NULL;

    SharedLibrary (std::string plugin_file) {
        so_file = plugin_file ;
    }

    //> Returns NULL if ok, error otherwise
    char * load (void) {
        dl_handle = dlopen (so_file.c_str(), RTLD_LAZY);
        if (dl_handle == NULL) {
            char * err = dlerror () ;
            LOGE ("Failed to load library: %s\n", err);
            return err ;
        }

        return NULL ;
    }
};

#endif // SHARED_LIBRARY_H
//
// Created by zerdzhong on 2019/10/12.
//

#ifndef FFBASE_UNIQUE_FD_H
#define FFBASE_UNIQUE_FD_H

#include "unique_object.h"
#include <unistd.h>

namespace ffbase {
namespace internal {

    struct UniqueFDTraits {
        static int InvalidValue() { return -1; }
        static bool IsValid(int value) { return value >= 0; }
        static void Free(int fd) { close(fd); };
    };

}  //end of namespace internal


using UniqueFD = UniqueObject<int, internal::UniqueFDTraits>;

}//end of namespace ffbase

#endif //FFBASE_UNIQUE_FD_H

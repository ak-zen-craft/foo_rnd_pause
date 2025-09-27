// pch.h: precompiled Headerfile.

// The files listed below will be compiled only once to improve build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code-searching features.
// However, the files listed here will ALL be recompiled if at least one of them is updated between builds.
// Do not add files here that should be updated frequently, as this will negate the performance benefit.

#ifndef PCH_H
#define PCH_H

// Add headers to be precompiled here.
#include "lib/foobar2000/SDK/foobar2000.h"
#ifdef __cplusplus
#include <helpers/foobar2000+atl.h>
#endif
#endif //PCH_H
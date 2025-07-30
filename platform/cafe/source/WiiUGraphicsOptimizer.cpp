// Implementation file for WiiUGraphicsOptimizer
// Contains static member definitions to avoid multiple definition errors

#ifdef __WIIU__

#include "WiiUGraphicsOptimizer.hpp"

// Static member definitions
int WiiUGraphicsOptimizer::frameDrawCallCount = 0;
int WiiUGraphicsOptimizer::lastPrepareDrawFrame = -1;
bool WiiUGraphicsOptimizer::emergencySkipMode = false;

#endif // __WIIU__

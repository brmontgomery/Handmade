#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "EngineSrc/Core/Core.h"
#include "EngineSrc/Core/Logging/Log.h"
#include "EngineSrc/Core/Debug/Instrumentor.h"
//#include "ES/Core/EntryPoint.h"

#ifdef HANDMADE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
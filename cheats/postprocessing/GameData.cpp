#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <list>
#include <memory>
#include <unordered_map>

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#ifdef _WIN32
#include "imgui/imgui_impl_dx9.h"
#else
#include "imgui/imgui_impl_opengl3.h"
#endif

#define STBI_ONLY_PNG
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "GameData.h"


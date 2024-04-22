#ifndef LOAD_VIDEO_H_
#define LOAD_VIDEO_H_

#include <vector>
#include "agl/image.h"

bool LoadVideo(const std::string& filename, std::vector<agl::Image>& frames); 

#endif

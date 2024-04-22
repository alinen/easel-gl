#include "loadvideo.h"

int main(int argc, char **argv)
{
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        exit(0);
    }
    const char* filename    = argv[1];

    std::vector<agl::Image> frames;
    if (LoadVideo(filename, frames)) {
      std::string outname = filename;
      outname += "-0.ppm";
      frames[0].save(outname, true);
    }
    else {
      printf("Could not open video: %s\n", filename);
    }

    return 0;
}
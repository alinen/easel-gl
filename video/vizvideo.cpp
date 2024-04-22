#include "agl/window.h"
#include "agl/image.h"
#include "loadvideo.h"
#include <glm/gtc/type_ptr.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

// Width and height should match image size
unsigned int WIDTH = 720;
unsigned int HEIGHT = 480;

class VideoViewer : public agl::Window {
 public:
  VideoViewer(const std::string& filename, float fps = 24.0f) {
    LoadVideo(filename, _images);
    _dt = 1.0/fps;
    _duration = _images.size() * _dt;
  }

  void setup() {
    _imageCurrent = 0; // force load on first frame
    setWindowSize(WIDTH, HEIGHT);
    
    setCameraEnabled(false);
    background(vec3(0.0f));
    loadImage();

    renderer.loadShader("screeneffects",
      "../shaders/shadertoy.vs",
      "../shaders/screeneffects.fs");

    renderer.loadRenderTexture("SceneRender", 0, 512, 512);
  }

  void loadImage() {
    if (_images.size() == 0) return;
    renderer.loadTexture("video", _images[_current], 1);  
    _imageCurrent = _current;
  }

  void draw() {
    if (_paused) {
      _time = _current * _dt;
    } 
    else {
      _time += dt();
      float u = fmod(_time, _duration) / _duration;
      _current = (int)(u * _images.size());
    }

    if (_current != _imageCurrent) {
      loadImage();
    }

    renderer.beginShader("screeneffects");
    lookAt(vec3(0), vec3(0, 0, -2));
    ortho(0, width(), 0, height(), -1, 1);
    background(vec3(0));
    renderer.setUniform("EffectType", _effect);
    renderer.setUniform("GlobalTime", elapsedTime());
    renderer.setUniform("Resolution", vec2(width(), height()));
    renderer.texture("ScreenTexture", "video");
    renderer.identity();
    renderer.translate(vec3(width()*0.5, height()*0.5, 0.0));
    renderer.scale(vec3(width(), height(), 1.0f));
    renderer.rotate(kPI / 2, vec3(1, 0, 0));
    renderer.plane();
    renderer.endShader();
    
    renderer.text(_paused ? "Paused" : "Playing", 10, 15);
    renderer.text("Current frame: " + std::to_string(_current), 10, 35);
  }

  void keyUp(int key, int mods) {
    if (key == '0') _effect = 0;
    else if (key == '1') _effect = 1;
    else if (key == '2') _effect = 2;
    else if (key == '3') _effect = 3;
    else if (key == '4') _effect = 4;
    else if (key == '5') _effect = 5;
    else if (key == '6') _effect = 6;
    else if (key == ' ') {
      _paused = !_paused;
    }
    else if (_paused && key == GLFW_KEY_UP) {
      _current = (_current + 1) % _images.size();
    }
    else if (_paused && key == GLFW_KEY_DOWN) {
      _current = _current - 1;
      if (_current < 0) {
        _current = _images.size() - 1;
      }
    }
  }

  void mouseDown(int button, int mods) {
  }

 private:
  std::vector<agl::Image> _images;
  int _current = 0;
  int _imageCurrent = -1;
  bool _paused = false;
  float _time = 0;
  float _dt = 0.03f;
  float _duration = 0;
  int _effect = 0;
};

int main(int argc, char** argv)
{
  std::string video = "../textures/jellyfish-ldc.mp4";
  std::cout << "Video: " << video << std::endl;
  VideoViewer viewer(video);
  viewer.run();
}

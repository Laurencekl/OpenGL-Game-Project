#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <random>
#include <vector>

#include "font.h"
#include "glb_model.h"

const GLfloat kCameraSpeed = 0.02;
const GLfloat kMouseSensitivity = 0.1;
const GLfloat kWorldScale = 1.20f;

const GLfloat kDefaultCameraEyePositionX = 10;
const GLfloat kDefaultCameraEyePositionY = 6;
const GLfloat kDefaultCameraEyePositionZ = 6;
GLfloat camera_eye_x = kDefaultCameraEyePositionX;
GLfloat camera_eye_y = kDefaultCameraEyePositionY;
GLfloat camera_eye_z = kDefaultCameraEyePositionZ;

const GLfloat kDefaultCameraDirectionX = -0.5817f;
const GLfloat kDefaultCameraDirectionY = -0.2053f;
const GLfloat kDefaultCameraDirectionZ = -0.7871f;
GLfloat camera_direction_x = kDefaultCameraDirectionX;
GLfloat camera_direction_y = kDefaultCameraDirectionY;
GLfloat camera_direction_z = kDefaultCameraDirectionZ;

const GLfloat kCameraUpX = 0;
const GLfloat kCameraUpY = 1;
const GLfloat kCameraUpZ = 0;

GLfloat camera_right_x = -0.7871f;
GLfloat camera_right_y = 0;
GLfloat camera_right_z = 0.5817f;

const GLfloat kDefaultCameraYaw = -126.47f;
GLfloat camera_yaw = kDefaultCameraYaw;

const GLfloat kCameraPitchLimit = 90;
const GLfloat kDefaultCameraPitch = -11.85f;
GLfloat camera_pitch = kDefaultCameraPitch;

const GLfloat kPerspectiveFieldOfViewAngle = 45;
const GLfloat kPerspectiveNearZ = 0.1;
const GLfloat kPerspectiveFarZ = 100;

const GLfloat kModelInitialRotationX = 0;
const GLfloat kModelInitialRotationY = 0;
const GLfloat kParkedCarPositionX = 3.4f;
const GLfloat kParkedCarPositionZ = -12.0f;

GLfloat translate_x = 0;
GLfloat translate_y = 0;
GLfloat translate_z = 0;
GLfloat rotate_x = 0;
GLfloat rotate_y = 0;
GLfloat rotate_z = 0;
GLfloat scale_x = 1;
GLfloat scale_y = 1;
GLfloat scale_z = 1;

const GLfloat kTranslateIncrement = 0.03;
const GLfloat kRotateIncrement = 0.25;
const GLfloat kScaleIncrement = 0.03;
const GLfloat kMinimumScale = 0.1;

bool translating = false;
bool rotating = false;
bool scaling = false;
bool street_light_enabled = false;
bool f_key_was_pressed = false;

GlbModel model;

GlbModel street_lamp_model;
GlbModel traffic_cone_model;
GlbModel house_model;
GlbModel neymar_model;
GlbModel umbrella_model;
GlbModel beetle_model;
GlbModel cloud_model;
GlbModel pikachu_model;

GlbTexture road_texture;

const GLfloat kRoadHeight = -0.65f;
const GLfloat kLampScale = 0.85f;
const GLfloat kConeScale = 0.16f;
const GLfloat kHouseScale = 1.7f;
const GLfloat kCarRoadOffset = 0.40f;
const GLfloat kSidewalkHeight = kRoadHeight + 0.18f;
const GLfloat kGrassHeight = kRoadHeight + 0.14f;
GLfloat house_ground_y = 0;
GLfloat lamp_ground_y = 0;
GLfloat cone_ground_y = 0;
const GLfloat kNeymarScale = 0.42f;
const GLfloat kUmbrellaScale = 0.80f;
const GLfloat kBeetleScale = 0.85f;
const GLfloat kPikachuScale = 0.16f;
GLfloat neymar_ground_y = 0;
GLfloat beetle_ground_y = 0;
GLfloat pikachu_ground_y = 0;

const GLfloat kStreetLightHeight = 4.8f;

struct StreetLampPosition {
  GLfloat x;
  GLfloat z;
  GLfloat rotation_y;
};

const StreetLampPosition street_lamps[] = {
  {5.9f, -11.5f, 0},
  {-5.9f, -11.5f, 180},
  {5.9f, 1.0f, 0},
  {-5.9f, 1.0f, 180},
  {5.9f, -25.0f, 0},
  {-5.9f, -25.0f, 180}
};
const int kStreetLampCount = sizeof(street_lamps) / sizeof(street_lamps[0]);

const GLfloat kHudHeight = 500;
const GLfloat kHudWidth = 500;
std::ostringstream hud_text;

double last_time = 0;
bool bright_mode_enabled = false;
bool b_key_was_pressed = false;
const int kRainDropCount = 1600;

struct MovingCloud {
  GLfloat x, y, z, scale, speed;
};
MovingCloud clouds[] = {
  {-14, 9.5f, -25, 2.2f, 0.65f},
  {4, 10.5f, -32, 2.6f, 0.50f},
  {-8, 14, -8, 2.1f, 0.75f},
  {14, 12, -15, 2.4f, 0.60f},
  {-20, 13, -38, 2.4f, 0.55f}
};
const int kCloudCount = sizeof(clouds) / sizeof(clouds[0]);
GLfloat cloud_bottom_y = -1.35f;

struct RainDrop {
  GLfloat offset_x, y, offset_z, speed, length;
  int cloud_index;
};
std::vector<RainDrop> rain_drops;
std::mt19937 weather_random(13);

GLfloat random_weather_value(GLfloat minimum, GLfloat maximum) {
  return std::uniform_real_distribution<GLfloat>(minimum, maximum)(weather_random);
}

void reset_rain_drop(RainDrop& drop, bool distribute_height = false) {
  const MovingCloud& cloud = clouds[drop.cloud_index];
  const GLfloat angle = random_weather_value(0, 6.2831853f);
  const GLfloat radius = std::sqrt(random_weather_value(0, 1));
  drop.offset_x = std::cos(angle) * radius * cloud.scale * 1.25f;
  drop.offset_z = std::sin(angle) * radius * cloud.scale * 2.10f;
  const GLfloat top = fmaxf(kRoadHeight + 0.1f,
      cloud.y + cloud_bottom_y * cloud.scale - drop.length - 0.05f);
  drop.y = distribute_height ? random_weather_value(kRoadHeight, top) : top;
}

void initialize_weather() {
  rain_drops.resize(kRainDropCount);
  for (int i = 0; i < kRainDropCount; ++i) {
    RainDrop& drop = rain_drops[i];
    drop.cloud_index = i % kCloudCount;
    drop.speed = random_weather_value(9, 14);
    drop.length = random_weather_value(0.22f, 0.40f);
    reset_rain_drop(drop, true);
  }
}

bool key_pressed_once(GLFWwindow* window, int key, bool& was_pressed) {
  const bool pressed = glfwGetKey(window, key) == GLFW_PRESS;
  const bool just_pressed = pressed && !was_pressed;
  was_pressed = pressed;
  return just_pressed;
}

static void mouse_read(GLFWwindow* window, double mouse_x, double mouse_y) {
  static bool mouse_initialized = false;
  static GLfloat last_mouse_x = 0, last_mouse_y = 0;
  if (!mouse_initialized) {
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;
    mouse_initialized = true;
  }

  GLfloat mouse_delta_x = (mouse_x - last_mouse_x);
  GLfloat mouse_delta_y = (last_mouse_y - mouse_y);
  last_mouse_x = mouse_x;
  last_mouse_y = mouse_y;

  camera_pitch += (mouse_delta_y * kMouseSensitivity);
  if (camera_pitch > kCameraPitchLimit) {
    camera_pitch = kCameraPitchLimit;
  }
  if (camera_pitch < -kCameraPitchLimit) {
    camera_pitch = -kCameraPitchLimit;
  }
  camera_yaw += (mouse_delta_x * kMouseSensitivity);

  GLfloat camera_pitch_radians = camera_pitch * (3.1415 / 180);
  GLfloat camera_yaw_radians = camera_yaw * (3.1415 / 180);
  camera_direction_x = cosf(camera_yaw_radians) * cosf(camera_pitch_radians);
  camera_direction_y = sinf(camera_pitch_radians);
  camera_direction_z = sinf(camera_yaw_radians) * cosf(camera_pitch_radians);

  camera_right_x = camera_direction_z * kCameraUpY - camera_direction_y * kCameraUpZ;
  camera_right_y = camera_direction_x * kCameraUpZ - camera_direction_z * kCameraUpX;
  camera_right_z = camera_direction_y * kCameraUpX - camera_direction_x * kCameraUpY;
}

void keyboard_read(GLFWwindow* window) {
  hud_text.str("");

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  const bool f_key_is_pressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
  if (f_key_is_pressed && !f_key_was_pressed) {
    street_light_enabled = !street_light_enabled;
    hud_text << (street_light_enabled ? "Poste ligado " : "Poste desligado ");
  }
  f_key_was_pressed = f_key_is_pressed;

  if (key_pressed_once(window, GLFW_KEY_B, b_key_was_pressed)) {
    bright_mode_enabled = !bright_mode_enabled;
  }
  hud_text << "B: claro " << (bright_mode_enabled ? "ON" : "OFF")
           << " | F: postes " << (street_light_enabled ? "ON " : "OFF ");

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera_eye_x += camera_direction_x * kCameraSpeed;
    camera_eye_y += camera_direction_y * kCameraSpeed;
    camera_eye_z += camera_direction_z * kCameraSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera_eye_x -= camera_direction_x * kCameraSpeed;
    camera_eye_y -= camera_direction_y * kCameraSpeed;
    camera_eye_z -= camera_direction_z * kCameraSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera_eye_x += camera_right_x * kCameraSpeed;
    camera_eye_y += camera_right_y * kCameraSpeed;
    camera_eye_z += camera_right_z * kCameraSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera_eye_x -= camera_right_x * kCameraSpeed;
    camera_eye_y -= camera_right_y * kCameraSpeed;
    camera_eye_z -= camera_right_z * kCameraSpeed;
  }

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera_eye_x = kDefaultCameraEyePositionX;
    camera_eye_y = kDefaultCameraEyePositionY;
    camera_eye_z = kDefaultCameraEyePositionZ;
    camera_direction_x = kDefaultCameraDirectionX;
    camera_direction_y = kDefaultCameraDirectionY;
    camera_direction_z = kDefaultCameraDirectionZ;
    camera_right_x = kDefaultCameraDirectionZ;
    camera_right_y = 0;
    camera_right_z = -kDefaultCameraDirectionX;
    camera_yaw = kDefaultCameraYaw;
    camera_pitch = kDefaultCameraPitch;

    translate_x = translate_y = translate_z = 0;
    rotate_x = rotate_y = rotate_z = 0;
    scale_x = scale_y = scale_z = 1;

    hud_text << "Redefinido! ";
  }

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    translating = true;
    hud_text << "Translacao ativa ";

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      translate_x -= kTranslateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      translate_x += kTranslateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      translate_y -= kTranslateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      translate_y += kTranslateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
      translate_z -= kTranslateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
      translate_z += kTranslateIncrement;
    }
  }
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
    translating = false;
  }

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    rotating = true;
    hud_text << "Rotacao ativa ";

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      rotate_x -= kRotateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      rotate_x += kRotateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      rotate_y -= kRotateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      rotate_y += kRotateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
      rotate_z -= kRotateIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
      rotate_z += kRotateIncrement;
    }
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
    rotating = false;
  }

  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    scaling = true;
    hud_text << "Escala ativa ";

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      scale_x = fmaxf(kMinimumScale, scale_x - kScaleIncrement);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      scale_x += kScaleIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      scale_y = fmaxf(kMinimumScale, scale_y - kScaleIncrement);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      scale_y += kScaleIncrement;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
      scale_z = fmaxf(kMinimumScale, scale_z - kScaleIncrement);
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
      scale_z += kScaleIncrement;
    }
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
    scaling = false;
  }

}

void animate() {
  const double current_time = glfwGetTime();
  const double delta_time = current_time - last_time;
  last_time = current_time;

  for (MovingCloud& cloud : clouds) {
    cloud.x += cloud.speed * delta_time;
    if (cloud.x > 30) cloud.x = -30 + std::fmod(cloud.x - 30, 60.0f);
  }
  for (RainDrop& drop : rain_drops) {
    drop.y -= drop.speed * delta_time;
    if (drop.y < kRoadHeight) {
      reset_rain_drop(drop);
    }
  }
}

void draw_rain() {
  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT |
               GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);
  glLineWidth(1);
  glColor4f(0.66f, 0.78f, 0.90f, 0.65f);
  glBegin(GL_LINES);
  for (const RainDrop& drop : rain_drops) {
    const MovingCloud& cloud = clouds[drop.cloud_index];
    const GLfloat x = cloud.x + drop.offset_x;
    const GLfloat z = cloud.z + drop.offset_z;
    glVertex3f(x, drop.y, z);
    glVertex3f(x + 0.04f, drop.y + drop.length, z);
  }
  glEnd();
  glPopAttrib();
}

void resize_window(GLFWwindow* window) {
  int window_width, window_height;
  glfwGetFramebufferSize(window, &window_width, &window_height);
  glViewport(0, 0, window_width, window_height);
}

void draw_ground_tile(GLfloat x1, GLfloat z1, GLfloat x2, GLfloat z2, GLfloat y,
                      GLfloat texture_scale = 1) {
  glNormal3f(0, 1, 0);
  glTexCoord2f(x1 * texture_scale, z1 * texture_scale); glVertex3f(x1, y, z1);
  glTexCoord2f(x1 * texture_scale, z2 * texture_scale); glVertex3f(x1, y, z2);
  glTexCoord2f(x2 * texture_scale, z2 * texture_scale); glVertex3f(x2, y, z2);
  glTexCoord2f(x2 * texture_scale, z1 * texture_scale); glVertex3f(x2, y, z1);
}

void draw_sidewalk(GLfloat left, GLfloat right) {
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  for (int z = -38; z < 8; ++z) {
    glColor3f(0.48f, 0.49f, 0.50f);
    draw_ground_tile(left, z, right, z + 1, kSidewalkHeight);
    glColor3f(0.68f, 0.69f, 0.70f);
    draw_ground_tile(left + 0.025f, z + 0.025f, right - 0.025f, z + 0.975f,
                     kSidewalkHeight + 0.002f);
  }
  glColor3f(0.56f, 0.57f, 0.58f);
  glNormal3f(-1, 0, 0);
  glVertex3f(left, kRoadHeight, -38); glVertex3f(left, kRoadHeight, 8);
  glVertex3f(left, kSidewalkHeight, 8); glVertex3f(left, kSidewalkHeight, -38);
  glNormal3f(1, 0, 0);
  glVertex3f(right, kRoadHeight, 8); glVertex3f(right, kRoadHeight, -38);
  glVertex3f(right, kSidewalkHeight, -38); glVertex3f(right, kSidewalkHeight, 8);
  glEnd();
}

void draw_grass(GLfloat left, GLfloat right) {
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  for (GLfloat x = left; x < right; x += 0.5f) {
    for (GLfloat z = -38; z < 8; z += 0.5f) {
      const int tile = static_cast<int>((x + 24) * 2) * 73 + static_cast<int>((z + 38) * 2) * 37;
      const GLfloat shade = (tile % 11) * 0.003f;
      glColor3f(0.20f + shade, 0.43f + shade, 0.12f + shade);
      draw_ground_tile(x, z, fminf(x + 0.5f, right), z + 0.5f, kGrassHeight);
    }
  }
  glEnd();
}

void draw_road() {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, road_texture.id);
  glColor3ub(255, 255, 255);

  glBegin(GL_QUADS);
  for (GLfloat x = -5; x < 5; x += 0.5f) {
    for (GLfloat z = -38; z < 8; z += 0.5f) {
      draw_ground_tile(x, z, x + 0.5f, z + 0.5f, kRoadHeight, 0.22f);
    }
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glColor3f(0.95f, 0.78f, 0.18f);
  for (int z = -37; z < 7; z += 4) {
    draw_ground_tile(-0.06f, z, 0.06f, z + 2, kRoadHeight + 0.005f);
  }
  glColor3f(0.9f, 0.9f, 0.86f);
  for (int z = -38; z < 8; ++z) {
    draw_ground_tile(-4.7f, z, -4.6f, z + 1, kRoadHeight + 0.005f);
    draw_ground_tile(4.6f, z, 4.7f, z + 1, kRoadHeight + 0.005f);
  }
  glEnd();
  draw_sidewalk(-7.5f, -5.0f);
  draw_sidewalk(5.0f, 7.5f);
  draw_grass(-22.0f, -7.5f);
  draw_grass(7.5f, 22.0f);
  glColor3f(0.67f, 0.65f, 0.60f);
  glBegin(GL_QUADS);
  for (GLfloat x = -14; x < -7.5f; x += 0.5f) {
    draw_ground_tile(x, -18.0f, x + 0.5f, -16.4f, kSidewalkHeight + 0.004f);
  }
  glEnd();
}

GLfloat model_ground_position(const GlbModel& scene_model, GLfloat scale, GLfloat ground) {
  GLfloat minimum_y = std::numeric_limits<GLfloat>::max();
  for (unsigned int i = 0; i < scene_model.scene->mNumMeshes; ++i) {
    const aiMesh* mesh = scene_model.scene->mMeshes[i];
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
      minimum_y = fminf(minimum_y, mesh->mVertices[j].y);
    }
  }
  return ground - (minimum_y - scene_model.center_y) * scene_model.normalization_scale * scale;
}

void draw_scene_object(const GlbModel& scene_model, GLfloat x, GLfloat y, GLfloat z,
                       GLfloat scale, GLfloat rotation_x = 0, GLfloat rotation_y = 0) {
  glPushMatrix();
  glTranslatef(x, y, z);
  glRotatef(rotation_x, 1, 0, 0);
  glRotatef(rotation_y, 0, 1, 0);
  glScalef(scale, scale, scale);

  draw_model_display_list(scene_model);
  glPopMatrix();
}

void draw_neymar_with_umbrella() {
  glPushMatrix();
  glTranslatef(-7.9f, neymar_ground_y, -17.2f);
  glRotatef(90, 0, 1, 0);
  glScalef(kNeymarScale, kNeymarScale, kNeymarScale);
  draw_model_display_list(neymar_model);

  glTranslatef(-0.597523f, 1.05617f, 0.633052f);
  glScalef(kUmbrellaScale, kUmbrellaScale, kUmbrellaScale);
  glTranslatef(umbrella_model.center_x * umbrella_model.normalization_scale,
               (umbrella_model.center_y - 0.10f) * umbrella_model.normalization_scale,
               umbrella_model.center_z * umbrella_model.normalization_scale);
  draw_model_display_list(umbrella_model);
  glPopMatrix();
}

void draw(GLFWwindow* window) {
  int window_width, window_height;
  glfwGetFramebufferSize(window, &window_width, &window_height);
  GLdouble aspect_ratio = (GLdouble)window_width / window_height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  GLdouble field_of_view = kPerspectiveFieldOfViewAngle;
  GLdouble near = kPerspectiveNearZ;
  GLdouble far = kPerspectiveFarZ;
  gluPerspective(field_of_view, aspect_ratio, near, far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  GLfloat camera_center_x = (camera_eye_x + camera_direction_x);
  GLfloat camera_center_y = (camera_eye_y + camera_direction_y);
  GLfloat camera_center_z = (camera_eye_z + camera_direction_z);
  gluLookAt(camera_eye_x, camera_eye_y, camera_eye_z, camera_center_x, camera_center_y,
            camera_center_z, kCameraUpX, kCameraUpY, kCameraUpZ);

  const GLfloat normal_ambient[] = {0.20f, 0.20f, 0.20f, 1};
  const GLfloat bright_ambient[] = {0.65f, 0.65f, 0.65f, 1};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,
                bright_mode_enabled ? bright_ambient : normal_ambient);
  const GLfloat sky_direction[] = {-0.4f, 1, 0.3f, 0};
  glLightfv(GL_LIGHT7, GL_POSITION, sky_direction);

  glPushMatrix();
  glScalef(kWorldScale, kWorldScale, kWorldScale);

  for (int i = 0; i < kStreetLampCount; ++i) {
    const GLenum light = GL_LIGHT0 + i;
    if (street_light_enabled) {
      glEnable(light);
      GLfloat position[] = {street_lamps[i].x, kStreetLightHeight,
                            street_lamps[i].z, 1.0f};
      glLightfv(light, GL_POSITION, position);
    } else {
      glDisable(light);
    }
  }

  glPushAttrib(GL_LIGHTING_BIT);
  const GLfloat cloud_emission[] = {0.18f, 0.20f, 0.22f, 1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, cloud_emission);
  for (const MovingCloud& cloud : clouds) {
    draw_scene_object(cloud_model, cloud.x, cloud.y, cloud.z, cloud.scale);
  }
  glPopAttrib();
  draw_road();
  for (const StreetLampPosition& lamp : street_lamps) {
    draw_scene_object(street_lamp_model, lamp.x, lamp_ground_y, lamp.z,
                      kLampScale, 0, lamp.rotation_y);
  }
  draw_scene_object(traffic_cone_model, 4.2f, cone_ground_y, -4.5f, kConeScale);
  draw_scene_object(house_model, -14.0f, house_ground_y, -19.0f, kHouseScale, 0, 90);
  draw_scene_object(beetle_model, -13.0f, beetle_ground_y, -12.3f, kBeetleScale, 0, -90);

  draw_neymar_with_umbrella();
  draw_scene_object(pikachu_model, -8.1f, pikachu_ground_y, -15.8f, kPikachuScale, 0, 90);

  glPushMatrix();
  glTranslatef(kParkedCarPositionX + translate_x, translate_y,
               kParkedCarPositionZ + translate_z);
  glRotatef(rotate_x, 1, 0, 0);
  glRotatef(rotate_y, 0, 1, 0);
  glRotatef(rotate_z, 0, 0, 1);
  glScalef(scale_x, scale_y, scale_z);

  glTranslatef(0, kCarRoadOffset, 0);
  glRotatef(kModelInitialRotationX, 1, 0, 0);
  glRotatef(kModelInitialRotationY, 0, 1, 0);

  glEnable(GL_TEXTURE_2D);
  draw_model_display_list(model);
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
  draw_rain();
  glPopMatrix();
}

void draw_hud(GLFWwindow* window) {
  int window_width, window_height;
  glfwGetFramebufferSize(window, &window_width, &window_height);
  GLdouble aspect_ratio = (GLdouble)window_width / window_height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLdouble left = 0;
  GLdouble right = kHudWidth;
  GLdouble bottom = 0;
  GLdouble top = kHudHeight;
  if (window_width > window_height) {
    gluOrtho2D((left * aspect_ratio), (right * aspect_ratio), bottom, top);
  } else {
    gluOrtho2D(left, right, (bottom / aspect_ratio), (top / aspect_ratio));
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  {
    glColor3ub(255, 255, 255);
    draw_text(5, 5, hud_text.str());

    glPointSize(20);
    glBegin(GL_POINTS);
    if (translating) {
      glColor3ub(255, 0, 0);
      glVertex2f(7.5, 20);
    }
    if (rotating) {
      glColor3ub(0, 255, 0);
      glVertex2f(17.5, 20);
    }
    if (scaling) {
      glColor3ub(0, 128, 255);
      glVertex2f(27.5, 20);
    }
    glEnd();
  }
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LIGHTING);
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Falha ao inicializar GLFW" << std::endl;
    return EXIT_FAILURE;
  }

  GLFWwindow* window = glfwCreateWindow(800, 800, "", NULL, NULL);
  if (!window) {
    std::cerr << "Falha ao criar a janela GLFW" << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwSetWindowPos(window, 0, 0);
  glfwSetWindowTitle(window, "Carregando modelo 3D... aguarde");
  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Falha ao inicializar GLEW" << std::endl;
    return EXIT_FAILURE;
  }

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  glEnable(GL_NORMALIZE);

  GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat light_diffuse[] = {1.15f, 1.00f, 0.78f, 1.0f};
  for (int i = 0; i < kStreetLampCount; ++i) {
    const GLenum light = GL_LIGHT0 + i;
    glLightfv(light, GL_AMBIENT, light_ambient);
    glLightfv(light, GL_DIFFUSE, light_diffuse);
    glLightf(light, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(light, GL_LINEAR_ATTENUATION, 0.10f / kWorldScale);
    glLightf(light, GL_QUADRATIC_ATTENUATION, 0.045f / (kWorldScale * kWorldScale));
  }
  const GLfloat sky_diffuse[] = {0.40f, 0.43f, 0.48f, 1};
  glLightfv(GL_LIGHT7, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT7, GL_DIFFUSE, sky_diffuse);
  glEnable(GL_LIGHT7);
  glClearColor(0.32f, 0.53f, 0.70f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  resize_window(window);
  hud_text.str("");
  hud_text << "CARREGANDO MODELO 3D...";
  draw_hud(window);
  glfwSwapBuffers(window);
  glfwPollEvents();
  std::cout << "Carregando o modelo 3D, aguarde..." << std::endl;

  if (!load_model(model, "models/sports_car.glb")) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  if (!load_texture_external(road_texture, "textures/aerial_asphalt_01_diff_1k.jpg") ||
      !load_model(street_lamp_model, "models/street_lamp/Street_Light_A_02.obj") ||
      !load_model(traffic_cone_model, "models/traffic_cone.glb") ||
      !load_model(house_model, "models/house.glb") ||
      !load_model(neymar_model, "models/neymar_holding.glb") ||
      !load_model(umbrella_model, "models/umbrella/Umbrella.obj") ||
      !load_model(beetle_model, "models/beetle/VWbeetle.obj", false) ||
      !load_model(cloud_model, "models/cloud/cloud.obj", true, true) ||
      !load_model(pikachu_model, "models/pikachu/pikachu.obj", true, true)) {
    destroy_model(model);
    destroy_model(street_lamp_model);
    destroy_model(traffic_cone_model);
    destroy_model(house_model);
    destroy_model(neymar_model);
    destroy_model(umbrella_model);
    destroy_model(beetle_model);
    destroy_model(cloud_model);
    destroy_model(pikachu_model);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  house_ground_y = model_ground_position(house_model, kHouseScale, kGrassHeight);
  lamp_ground_y = model_ground_position(street_lamp_model, kLampScale, kSidewalkHeight);
  cone_ground_y = model_ground_position(traffic_cone_model, kConeScale, kRoadHeight);
  neymar_ground_y = model_ground_position(neymar_model, kNeymarScale, kSidewalkHeight + 0.004f);
  beetle_ground_y = model_ground_position(beetle_model, kBeetleScale, kGrassHeight);
  pikachu_ground_y = model_ground_position(pikachu_model, kPikachuScale, kGrassHeight);
  cloud_bottom_y = -model_ground_position(cloud_model, 1, 0);
  for (auto& material : cloud_model.material_colors) {
    material.second = aiColor4D(0.78f, 0.80f, 0.83f, 1);
  }

  if (!create_model_display_list(model) || !create_model_display_list(street_lamp_model) ||
      !create_model_display_list(traffic_cone_model) || !create_model_display_list(house_model) ||
      !create_model_display_list(neymar_model) || !create_model_display_list(umbrella_model) ||
      !create_model_display_list(beetle_model) || !create_model_display_list(cloud_model) ||
      !create_model_display_list(pikachu_model)) {
    destroy_model(model);
    destroy_model(street_lamp_model);
    destroy_model(traffic_cone_model);
    destroy_model(house_model);
    destroy_model(neymar_model);
    destroy_model(umbrella_model);
    destroy_model(beetle_model);
    destroy_model(cloud_model);
    destroy_model(pikachu_model);
    if (road_texture.id) glDeleteTextures(1, &road_texture.id);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  initialize_weather();
  last_time = glfwGetTime();
  glfwSetWindowTitle(window, "Aula 13 - Nuvens e chuva animadas");
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_read);

  while (!glfwWindowShouldClose(window)) {
    keyboard_read(window);
    resize_window(window);
    animate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw(window);
    draw_hud(window);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  destroy_model(model);
  destroy_model(street_lamp_model);
  destroy_model(traffic_cone_model);
  destroy_model(house_model);
  destroy_model(neymar_model);
  destroy_model(umbrella_model);
  destroy_model(beetle_model);
  destroy_model(cloud_model);
  destroy_model(pikachu_model);
  if (road_texture.id) glDeleteTextures(1, &road_texture.id);

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}

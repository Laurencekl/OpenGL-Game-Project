#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>

#include "font.h"
#include "glb_model.h"

const GLfloat kCameraSpeed = 0.02;
const GLfloat kMouseSensitivity = 0.1;

const GLfloat kDefaultCameraEyePositionX = 0;
const GLfloat kDefaultCameraEyePositionY = 0;
const GLfloat kDefaultCameraEyePositionZ = 0;
GLfloat camera_eye_x = kDefaultCameraEyePositionX;
GLfloat camera_eye_y = kDefaultCameraEyePositionY;
GLfloat camera_eye_z = kDefaultCameraEyePositionZ;

const GLfloat kDefaultCameraDirectionX = 0;
const GLfloat kDefaultCameraDirectionY = 0;
const GLfloat kDefaultCameraDirectionZ = -1;
GLfloat camera_direction_x = kDefaultCameraDirectionX;
GLfloat camera_direction_y = kDefaultCameraDirectionY;
GLfloat camera_direction_z = kDefaultCameraDirectionZ;

const GLfloat kCameraUpX = 0;
const GLfloat kCameraUpY = 1;
const GLfloat kCameraUpZ = 0;

GLfloat camera_right_x = -1;
GLfloat camera_right_y = 0;
GLfloat camera_right_z = 0;

const GLfloat kDefaultCameraYaw = -90;
GLfloat camera_yaw = kDefaultCameraYaw;

const GLfloat kCameraPitchLimit = 90;
const GLfloat kDefaultCameraPitch = 0;
GLfloat camera_pitch = kDefaultCameraPitch;

const GLfloat kPerspectiveFieldOfViewAngle = 45;
const GLfloat kPerspectiveNearZ = 0.1;
const GLfloat kPerspectiveFarZ = 100;
const GLfloat kPerspectiveTranslateZ = 10;

const GLfloat kModelInitialRotationX = 0;
const GLfloat kModelInitialRotationY = -30;

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

GlbModel model;

GlbModel street_lamp_model;
GlbModel traffic_cone_model;

GlbTexture road_texture;

const GLfloat kRoadHeight = -0.65f;
const GLfloat kLampScale = 0.85f;
const GLfloat kConeScale = 0.16f;
const GLfloat kCarRoadOffset = 0.40f;

const GLfloat kDefaultLightPositionX = 0.0f;
const GLfloat kDefaultLightPositionY = 4.0f;
const GLfloat kDefaultLightPositionZ = -8.0f;
const GLfloat kMinimumLightPositionY = 0.0f;
const GLfloat kLightSpeed = 0.08f;
GLfloat light_position_x = kDefaultLightPositionX;
GLfloat light_position_y = kDefaultLightPositionY;
GLfloat light_position_z = kDefaultLightPositionZ;

const GLfloat kHudHeight = 300;
const GLfloat kHudWidth = 300;
std::ostringstream hud_text;

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
    camera_right_x = -1;
    camera_right_y = 0;
    camera_right_z = 0;
    camera_yaw = kDefaultCameraYaw;
    camera_pitch = kDefaultCameraPitch;

    translate_x = translate_y = translate_z = 0;
    rotate_x = rotate_y = rotate_z = 0;
    scale_x = scale_y = scale_z = 1;

    light_position_x = kDefaultLightPositionX;
    light_position_y = kDefaultLightPositionY;
    light_position_z = kDefaultLightPositionZ;

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

  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      light_position_x -= kLightSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      light_position_x += kLightSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS &&
        light_position_y > kMinimumLightPositionY) {
      light_position_y -= kLightSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      light_position_y += kLightSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
      light_position_z -= kLightSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
      light_position_z += kLightSpeed;
    }
  }
}

void resize_window(GLFWwindow* window) {
  int window_width, window_height;
  glfwGetFramebufferSize(window, &window_width, &window_height);
  glViewport(0, 0, window_width, window_height);
}

void draw_road() {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, road_texture.id);
  glColor3ub(255, 255, 255);

  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);
  glTexCoord2f(0, 0);
  glVertex3f(-10, kRoadHeight, 8);
  glTexCoord2f(0, 10);
  glVertex3f(-10, kRoadHeight, -38);
  glTexCoord2f(4, 10);
  glVertex3f(10, kRoadHeight, -38);
  glTexCoord2f(4, 0);
  glVertex3f(10, kRoadHeight, 8);
  glEnd();

  glDisable(GL_TEXTURE_2D);
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

  GLfloat light_position[] = {light_position_x, light_position_y, light_position_z, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  draw_road();
  draw_scene_object(street_lamp_model, -7.2f, 1.9f, -11.5f, kLampScale, 0, 180);
  draw_scene_object(street_lamp_model, 7.2f, 1.9f, -11.5f, kLampScale);
  draw_scene_object(traffic_cone_model, 2.3f, -0.15f, -7.8f, kConeScale);

  glTranslatef(translate_x, translate_y, translate_z - kPerspectiveTranslateZ);
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

  GLfloat light_ambient[] = {0.20f, 0.20f, 0.20f, 1.0f};
  GLfloat light_diffuse[] = {0.90f, 0.90f, 0.90f, 1.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glClearColor(0.35f, 0.65f, 0.95f, 1.0f);

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
      !load_model(traffic_cone_model, "models/traffic_cone.glb")) {
    destroy_model(model);
    destroy_model(street_lamp_model);
    destroy_model(traffic_cone_model);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  if (!create_model_display_list(model) || !create_model_display_list(street_lamp_model) ||
      !create_model_display_list(traffic_cone_model)) {
    destroy_model(model);
    destroy_model(street_lamp_model);
    destroy_model(traffic_cone_model);
    if (road_texture.id) glDeleteTextures(1, &road_texture.id);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwSetWindowTitle(window, "Cena 3D com luz movel - Aula 12");
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_read);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    keyboard_read(window);
    resize_window(window);
    draw(window);
    draw_hud(window);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  destroy_model(model);
  destroy_model(street_lamp_model);
  destroy_model(traffic_cone_model);
  if (road_texture.id) glDeleteTextures(1, &road_texture.id);

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}

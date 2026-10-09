// Copyright
// Computação Gráfica
// URI Santiago
// Professor Laurence

#ifndef MODEL_HANDLER_H
#define MODEL_HANDLER_H

#include <GL/glew.h>

#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include "stb_image.h"

struct GlbTexture {
  GLuint id = 0;
  int width = 0;
  int height = 0;
};

// AULA 9: estrutura que mantem a cena, texturas e ajuste de tamanho do modelo.
struct GlbModel {
  std::unique_ptr<Assimp::Importer> importer;
  const aiScene* scene = nullptr;
  std::unordered_map<unsigned int, GlbTexture> textures;
  std::unordered_map<unsigned int, aiColor4D> material_colors;
  
  // AULA 10: ID da Display List que guarda os comandos de desenho compilados.
  GLuint display_list = 0;
  GLfloat center_x = 0;
  GLfloat center_y = 0;
  GLfloat center_z = 0;
  GLfloat normalization_scale = 1;
};

static inline bool load_texture_path_from_material(std::string& texture_path_string,
                                                   const aiMaterial* material) {
  if (!material) {
    return false;
  }

  aiString texture_path;
  if (material->GetTextureCount(aiTextureType_DIFFUSE) == 0 ||
      material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) != AI_SUCCESS) {
    return false;
  }

  texture_path_string = texture_path.C_Str();
  return !texture_path_string.empty();
}

// AULA 9: envia os pixels para o OpenGL e cria uma textura 2D com mipmaps.
static inline GlbTexture load_texture(const uint8_t* texture_data, GLenum internal_format,
                                      int texture_width, int texture_height, GLenum input_format) {
  GLint previous_texture_bind, previous_unpack_alignment;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture_bind);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &previous_unpack_alignment);

  GlbTexture texture;
  glGenTextures(1, &texture.id);
  glBindTexture(GL_TEXTURE_2D, texture.id);
  texture.width = texture_width;
  texture.height = texture_height;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture_width, texture_height, 0, input_format,
               GL_UNSIGNED_BYTE, texture_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glGenerateMipmap(GL_TEXTURE_2D);

  glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);
  glBindTexture(GL_TEXTURE_2D, previous_texture_bind);
  return texture;
}

// AULA 9: extrai e decodifica uma textura embutida dentro do arquivo GLB.
static inline bool load_texture_embedded(GlbTexture& glb_texture, const aiTexture* assimp_texture) {
  if (!assimp_texture) {
    return false;
  }

  const uint8_t* assimp_texture_data = reinterpret_cast<const uint8_t*>(assimp_texture->pcData);

  if (assimp_texture->mHeight == 0) {
    int texture_length = assimp_texture->mWidth;
    int texture_width;
    int texture_height;
    uint8_t* glb_texture_data = stbi_load_from_memory(assimp_texture_data, texture_length,
                                                      &texture_width, &texture_height, nullptr, 4);
    if (!glb_texture_data) {
      return false;
    }

    const GLenum internal_format = GL_RGBA;
    const GLenum input_format = GL_RGBA;
    glb_texture = load_texture(glb_texture_data, internal_format, texture_width, texture_height,
                               input_format);
    stbi_image_free(glb_texture_data);
    return true;
  }

  const GLenum internal_format = GL_RGBA;
  const int texture_width = assimp_texture->mWidth;
  const int texture_height = assimp_texture->mHeight;
  const GLenum input_format = GL_BGRA;
  glb_texture = load_texture(assimp_texture_data, internal_format, texture_width, texture_height,
                             input_format);
  return true;
}

static inline bool load_texture_external(GlbTexture& texture, const std::string& filepath) {
  int texture_width;
  int texture_height;
  int channels;
  uint8_t* texture_data =
      stbi_load(filepath.c_str(), &texture_width, &texture_height, &channels, 4);
  if (!texture_data) {
    std::cerr << "Textura nao encontrada: " << filepath << std::endl;
    return false;
  }

  texture = load_texture(texture_data, GL_RGBA, texture_width, texture_height, GL_RGBA);
  stbi_image_free(texture_data);
  return true;
}

// AULA 9: calcula o centro e uma escala padrao usando os limites dos vertices.
static inline void calculate_model_normalization(GlbModel& model) {
  aiVector3D minimum(std::numeric_limits<float>::max());
  aiVector3D maximum(std::numeric_limits<float>::lowest());

  for (unsigned int mesh_index = 0; mesh_index < model.scene->mNumMeshes; mesh_index++) {
    const aiMesh* mesh = model.scene->mMeshes[mesh_index];
    for (unsigned int vertex_index = 0; vertex_index < mesh->mNumVertices; vertex_index++) {
      const aiVector3D& point = mesh->mVertices[vertex_index];
      minimum.x = std::min(minimum.x, point.x);
      minimum.y = std::min(minimum.y, point.y);
      minimum.z = std::min(minimum.z, point.z);
      maximum.x = std::max(maximum.x, point.x);
      maximum.y = std::max(maximum.y, point.y);
      maximum.z = std::max(maximum.z, point.z);
    }
  }

  model.center_x = (minimum.x + maximum.x) * 0.5f;
  model.center_y = (minimum.y + maximum.y) * 0.5f;
  model.center_z = (minimum.z + maximum.z) * 0.5f;

  const GLfloat width = maximum.x - minimum.x;
  const GLfloat height = maximum.y - minimum.y;
  const GLfloat depth = maximum.z - minimum.z;
  const GLfloat largest_dimension = std::max(width, std::max(height, depth));
  if (largest_dimension > 0) {
    model.normalization_scale = 6.0f / largest_dimension;
  }
}

// AULA 9: carrega o GLB com Assimp, prepara as faces/normais/UVs e suas texturas.
static inline bool load_model(GlbModel& model, const std::string& filepath) {
  model.importer = std::make_unique<Assimp::Importer>();
  model.scene = model.importer->ReadFile(
      filepath.c_str(), aiProcess_Triangulate | aiProcess_GenNormals |
                            aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices |
                            aiProcess_CalcTangentSpace | aiProcess_FlipUVs |
                            aiProcess_PreTransformVertices);
  if (!model.scene) {
    std::cerr << "Erro ao carregar modelo: " << filepath << std::endl;
    std::cerr << model.importer->GetErrorString() << std::endl;
    return false;
  }

  model.textures.clear();
  model.material_colors.clear();
  calculate_model_normalization(model);
  const std::filesystem::path model_directory =
      std::filesystem::path(filepath).parent_path();

  for (unsigned int material_index = 0; material_index < model.scene->mNumMaterials;
       material_index++) {
    const aiMaterial* material = model.scene->mMaterials[material_index];

    // AULA 9: partes sem textura usam a cor armazenada no material.
    aiColor4D material_color(1.0f, 1.0f, 1.0f, 1.0f);
    if (material->Get(AI_MATKEY_BASE_COLOR, material_color) != AI_SUCCESS) {
      material->Get(AI_MATKEY_COLOR_DIFFUSE, material_color);
    }
    model.material_colors[material_index] = material_color;

    std::string texture_path_string;
    if (!load_texture_path_from_material(texture_path_string, material)) {
      continue;
    }

    GlbTexture texture;
    // AULA 9: "*numero" indica que a textura esta embutida no proprio GLB.
    if (texture_path_string[0] == '*') {
      const unsigned int texture_index =
          std::strtoul(texture_path_string.c_str() + 1, nullptr, 10);
      if (texture_index >= model.scene->mNumTextures ||
          !load_texture_embedded(texture, model.scene->mTextures[texture_index])) {
        continue;
      }
    } else {
      std::replace(texture_path_string.begin(), texture_path_string.end(), '\\', '/');
      std::filesystem::path texture_path(texture_path_string);
      if (texture_path.is_relative()) {
        texture_path = model_directory / texture_path;
      }

      if (!std::filesystem::exists(texture_path)) {
        texture_path = model_directory / "Textures" /
                       std::filesystem::path(texture_path_string).filename();
      }

      if (!load_texture_external(texture, texture_path.lexically_normal().string())) {
        continue;
      }
    }
    model.textures[material_index] = texture;
  }

  std::cout << "Modelo carregado: " << filepath << " (" << model.scene->mNumMeshes
            << " malhas, " << model.textures.size() << " texturas)" << std::endl;

  return true;
}

// AULA 9: percorre a arvore do modelo e desenha suas malhas como triangulos.
static inline void draw_model(const GlbModel& model, const aiNode* node) {
  if (!node) {
    return;
  }

  glPushMatrix();
  {
    aiMatrix4x4 matrix = node->mTransformation;
    matrix.Transpose();
    glMultMatrixf(&matrix.a1);

    for (unsigned int mesh_index = 0; mesh_index < node->mNumMeshes; mesh_index++) {
      const aiMesh* mesh = model.scene->mMeshes[node->mMeshes[mesh_index]];

      // AULA 9: usa a textura da malha ou, se ela nao existir, a cor do material.
      const auto iterator = model.textures.find(mesh->mMaterialIndex);
      if (iterator != model.textures.end() && iterator->second.id) {
        glColor3ub(255, 255, 255);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, iterator->second.id);

      } else {
        glDisable(GL_TEXTURE_2D);
        const auto color_iterator = model.material_colors.find(mesh->mMaterialIndex);
        if (color_iterator != model.material_colors.end()) {
          const aiColor4D& color = color_iterator->second;
          glColor4f(color.r, color.g, color.b, color.a);
        } else {
          glColor3ub(255, 255, 255);
        }
      }

      glBegin(GL_TRIANGLES);
      for (unsigned int face_index = 0; face_index < mesh->mNumFaces; face_index++) {
        const aiFace& face = mesh->mFaces[face_index];
        for (unsigned int vertex_index = 0; vertex_index < face.mNumIndices; vertex_index++) {
          const unsigned current_vertex_index = face.mIndices[vertex_index];

          if (mesh->HasNormals()) {
            const aiVector3D& normal = mesh->mNormals[current_vertex_index];
            glNormal3f(normal.x, normal.y, normal.z);
          }

          if (mesh->HasTextureCoords(0)) {
            const aiVector3D& texture_uv = mesh->mTextureCoords[0][current_vertex_index];
            glTexCoord2f(texture_uv.x, texture_uv.y);
          }

          const aiVector3D& point = mesh->mVertices[current_vertex_index];
          glVertex3f(point.x, point.y, point.z);
        }
      }
      glEnd();
    }

    for (unsigned int child_index = 0; child_index < node->mNumChildren; child_index++) {
      draw_model(model, node->mChildren[child_index]);
    }
  }
  glPopMatrix();
}

// AULA 9: centraliza/redimensiona o modelo e inicia o desenho pelo no raiz.
static inline void draw_model(const GlbModel& model) {
  if (!model.scene) {
    return;
  }
  if (!model.scene->mRootNode) {
    return;
  }

  glPushMatrix();
  glScalef(model.normalization_scale, model.normalization_scale,
           model.normalization_scale);
  glTranslatef(-model.center_x, -model.center_y, -model.center_z);
  draw_model(model, model.scene->mRootNode);
  glPopMatrix();
}

// AULA 10: DISPLAY LIST
// AULA 10: compila uma vez os comandos produzidos por draw_model.
static inline bool create_model_display_list(GlbModel& model) {
  if (!model.scene) {
    return false;
  }

  if (model.display_list) {
    glDeleteLists(model.display_list, 1);
  }

  // AULA 10: reserva o identificador da nova Display List.
  model.display_list = glGenLists(1);
  if (!model.display_list) {
    std::cerr << "Erro ao criar a display list do modelo." << std::endl;
    return false;
  }

  // AULA 10: grava os comandos de draw_model entre glNewList e glEndList.
  glNewList(model.display_list, GL_COMPILE);
  draw_model(model);
  glEndList();

  std::cout << "Display list criada: " << model.display_list << std::endl;
  return true;
}

// AULA 10: glCallList repete em cada quadro os comandos compilados anteriormente.
static inline void draw_model_display_list(const GlbModel& model) {
  if (model.display_list) {
    glCallList(model.display_list);
  }
}

// AULA 9 e 10: libera a Display List, as texturas e a cena do Assimp.
static inline void destroy_model(GlbModel& model) {
  if (model.display_list) {
    glDeleteLists(model.display_list, 1);
    model.display_list = 0;
  }

  for (auto& model_texture : model.textures) {
    if (model_texture.second.id) {
      glDeleteTextures(1, &model_texture.second.id);
      model_texture.second.id = 0;
    }
  }
  model.textures.clear();
  model.material_colors.clear();
  model.scene = nullptr;
  model.importer.reset();
}

#endif

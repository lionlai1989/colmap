// Copyright (c), ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "colmap/ui/triangle_painter.h"

#include "colmap/util/opengl_utils.h"

namespace colmap {

TrianglePainter::TrianglePainter() : num_geoms_(0) {}

TrianglePainter::~TrianglePainter() {
  vao_.destroy();
  vbo_.destroy();
}

void TrianglePainter::Setup() {
  vao_.destroy();
  vbo_.destroy();
  if (shader_program_.isLinked()) {
    shader_program_.release();
    shader_program_.removeAllShaders();
  }

  shader_program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                          ":/shaders/triangles.v.glsl");
  shader_program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                          ":/shaders/triangles.f.glsl");
  shader_program_.link();
  shader_program_.bind();

  vao_.create();
  vbo_.create();

#if DEBUG
  glDebugLog();
#endif
}

void TrianglePainter::Upload(const std::vector<TrianglePainter::Data>& data) {
  num_geoms_ = data.size();
  if (num_geoms_ == 0) {
    return;
  }

  vao_.bind();
  vbo_.bind();

  // Upload data array to GPU
  vbo_.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  vbo_.allocate(data.data(),
                static_cast<int>(data.size() * sizeof(TrianglePainter::Data)));

  // in_position
  shader_program_.enableAttributeArray("a_position");
  shader_program_.setAttributeBuffer(
      "a_position", GL_FLOAT, 0, 3, sizeof(PointPainter::Data));

  // in_color
  shader_program_.enableAttributeArray("a_color");
  shader_program_.setAttributeBuffer(
      "a_color", GL_FLOAT, 3 * sizeof(GLfloat), 4, sizeof(PointPainter::Data));

  // Make sure they are not changed from the outside
  vbo_.release();
  vao_.release();

#if DEBUG
  glDebugLog();
#endif
}

void TrianglePainter::Render(const QMatrix4x4& pmv_matrix) {
  if (num_geoms_ == 0) {
    return;
  }

  shader_program_.bind();
  vao_.bind();

  shader_program_.setUniformValue("u_pmv_matrix", pmv_matrix);

  QOpenGLFunctions* gl_funcs = QOpenGLContext::currentContext()->functions();
  gl_funcs->glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(3 * num_geoms_));

  // Make sure the VAO is not changed from the outside
  vao_.release();

#if DEBUG
  glDebugLog();
#endif
}

}  // namespace colmap

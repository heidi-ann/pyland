#include "RenderableComponent.h"

#define VERTEX_POS_INDX 0
#define  VERTEX_TEXCOORD0_INDX 1

RenderableComponent::RenderableComponent() {
  //Generate the vertex buffers
  glGenBuffers(1, &vbo_vertex_id);
  glGenBuffers(1, &vbo_texture_id);
}

RenderableComponent::~RenderableComponent() {
  //Delete the vertex buffers
  glDeleteBuffers(1, &vbo_vertex_id);
  glDeleteBuffers(1, &vbo_texture_id);
}

void RenderableComponent::set_vertex_data(GLfloat* new_vertex_data, int data_size, bool is_dynamic) { 
  vertex_data = new_vertex_data;
  vertex_data_size = data_size;

  //Set up buffer usage
  GLenum usage = GL_STATIC_DRAW;
  if(is_dynamic)
    usage = GL_DYNAMIC_DRAW;
  
  //Pass in data to the buffer buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex_id);
  glBufferData(GL_ARRAY_BUFFER, vertex_data_size, vertex_data, usage);

  //Set the attributes
  glEnableVertexAttribArray(VERTEX_POS_INDX);

  glVertexAttribPointer(VERTEX_POS_INDX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void RenderableComponent::set_texture_data(GLfloat* new_texture_data, int data_size, bool is_dynamic) { 
  texture_data = new_texture_data;
  texture_data_size = data_size;

  //Set up buffer usage
  GLenum usage = GL_STATIC_DRAW;
  if(is_dynamic)
    usage = GL_DYNAMIC_DRAW;
  
  //Pass in data to the buffer buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_id);
  glBufferData(GL_ARRAY_BUFFER, texture_data_size, texture_data, usage);

  //Set the attributes
  glEnableVertexAttribArray(VERTEX_TEXCOORD0_INDX);

  glVertexAttribPointer(VERTEX_TEXCOORD0_INDX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void RenderableComponent::bind_vbos() {

  //Bind the vertex data buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex_id);
  glVertexAttribPointer(0 /*VERTEX_POS_INDX*/, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0 /* VERTEX_POS_INDX */);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_id);
  glVertexAttribPointer(1 /* VERTEX_TEXCOORD0_INDX */, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray( 1 /*VERTEX_TEXCOORD0_INDX*/);

  glBindAttribLocation(shader->get_program(), 0 /*VERTEX_POS_INDX*/, "a_position");

  glBindAttribLocation(shader->get_program(), 1/*VERTEX_TEXCOORD0_INDX*/, "a_texCoord");


  //set sampler texture to unit 0
  glUniform1i(glGetUniformLocation(shader->get_program(), "s_texture"), 0);


}
void RenderableComponent::bind_textures() {
  glActiveTexture(GL_TEXTURE0);
  //Bind tiles texture
  glBindTexture(GL_TEXTURE_2D,texture_obj_id);
  
}

void RenderableComponent::release_textures() {
  glBindTexture(GL_TEXTURE_2D, 0);
}
void RenderableComponent::release_vbos() {
  glBindBuffer(GL_ARRAY_BUFFER, 0);

}


void RenderableComponent::bind_shader() {
  glUseProgram(shader->get_program());
}

void RenderableComponent::release_shader() {
  glUseProgram(0);
}

uniform mat4 mat_projection;
uniform mat4 mat_modelview;

attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
  gl_Position =  mat_projection * mat_modelview *  a_position;
  v_texCoord = a_texCoord;
}


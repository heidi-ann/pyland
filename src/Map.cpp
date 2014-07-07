#include "Map.h"


#include <string>
#include <iostream>
#include <fstream>
//Include GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#ifdef USE_GLES

#include <GLES2/gl2.h>

#endif

#ifdef USE_GL

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#endif

#include "game_window.hpp"


#define VERTEX_POS_INDX 0
#define VERTEX_TEXCOORD0_INDX 1
#define PATH "./"
#define GLOBAL_SCALE 2

#define IMAGE1_SIZE_WIDTH 128
#define IMAGE1_NUM_COMPONENTS 4
#define IMAGE1_SIZE_HEIGHT 240



#define IMAGE2_SIZE_WIDTH 192
#define IMAGE2_NUM_COMPONENTS 4
#define IMAGE2_SIZE_HEIGHT 128

#ifndef M_PI
#define M_PI 3.141592654
#endif

#define TILESET_ELEMENT_SIZE 16


/**
 * Constructor for Map
 */ 
Map::Map(const std::string map_src) : renderable_component() {


  init_shaders();
  generate_tileset_coords(IMAGE1_SIZE_WIDTH, IMAGE1_SIZE_HEIGHT);
  generate_map_texcoords();
  //  generate_sprite_tex_data();
  init_textures();
}

/** 
 *Destructor for Map
 */
Map::~Map() {
    // clear screen
    glClear( GL_COLOR_BUFFER_BIT );

    // release buffers
    delete[] tex_buf[0];
    delete[] tex_buf[1];
    delete[] map_data;
    delete[] sprite_data;
    delete[] sprite_tex_data;
    delete[] map_tex_coords;
    delete[] tileset_tex_coords;

    printf("\nClosed\n");
}
/**
 * Function used to generate the necessary Vertex Buffer Objects to
 * hold the map data to achieve more efficient rendering.
 */
void Map::init_vbo_buffer() {
    glUseProgram(program_obj);

    //vbo_ids[0] = map geometric data
    //vbo_ids[1] = tileset
    //vbo_ids[2] = sprite geometric data
    //vbo_ids[3] = texture coords for sprite
    glGenBuffers(num_vbo_ids, vbo_ids);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);
    glBufferData(GL_ARRAY_BUFFER, map_height*map_width*sizeof(GLfloat)*18, map_data, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[1]);
    glBufferData(GL_ARRAY_BUFFER, map_height*map_width*sizeof(GLfloat)*12, map_tex_coords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*18, sprite_data, GL_STATIC_DRAW);

    //changing texture coords
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*12, sprite_tex_data, GL_DYNAMIC_DRAW);

    //MAPS
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[0]);
    glEnableVertexAttribArray(VERTEX_POS_INDX);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[1]);
    glEnableVertexAttribArray(VERTEX_TEXCOORD0_INDX);


    //SPRITES
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[2]);
    glEnableVertexAttribArray(VERTEX_POS_INDX);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[3]);
    glEnableVertexAttribArray(VERTEX_TEXCOORD0_INDX);


    glVertexAttribPointer(VERTEX_POS_INDX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(VERTEX_TEXCOORD0_INDX, 2, GL_FLOAT, GL_TRUE, 0, nullptr);

    glActiveTexture(GL_TEXTURE0);

    //Bind tiles texture
    glBindTexture(GL_TEXTURE_2D,texture_ids[0]);

}

/**
 * The function used to generate the cache of tile texture coordinates.
 */ 
void Map::generate_tileset_coords(int tileset_width, int tileset_height) {
#ifdef DEBUG
    printf("GENERATING TILESET TEXTURE COORDS...");
#endif
    //check the tilset image height and widths are multiples of the tiles
    //  assert(image_height % TILESET_ELEMENT_SIZE != 0 || image_width % TILESET_ELEMENT_SIZE != 0);

    assert(TILESET_ELEMENT_SIZE != 0);

    int num_tiles_x = tileset_width  / TILESET_ELEMENT_SIZE;
    int num_tiles_y = tileset_height / TILESET_ELEMENT_SIZE;
    std::cout << num_tiles_x << " " << num_tiles_y << std::endl;

    assert(num_tiles_x);
    assert(num_tiles_y);

    //Each tile needs 8 floats to describe its position in the image
    tileset_tex_coords = new GLfloat[sizeof(GLfloat)* num_tiles_x * num_tiles_y * 4 * 2];
    assert(tileset_tex_coords);

    //Tiles are indexed from top left but Openl uses texture coordinates from bottom left
    //so we remap these 
    //We work from left to right, moving down
    GLfloat tileset_offset_x = 0.0;
    GLfloat tileset_offset_y = 0.0;
    GLfloat tileset_inc_x = GLfloat(1.0) / static_cast<GLfloat>(num_tiles_x);
    GLfloat tileset_inc_y = GLfloat(1.0) / static_cast<GLfloat>(num_tiles_y);
    //TODo: DIV ZEro HERRE 

    //TODO: REMEMBER TILESET COORDINATES ARE INVERSE OF IMAGE FILE ONES
    //generate the coordinates for each tile
    for (int y = 0; y < num_tiles_y; y++) {
        for (int x = 0; x < num_tiles_x; x++) {
            //bottom left
            tileset_tex_coords[y* num_tiles_x*4*2+x*(4*2)]  = tileset_offset_x;
            tileset_tex_coords[y* num_tiles_x*4*2+x*4*2 +1] = tileset_offset_y + tileset_inc_y;
 
            //top left
            tileset_tex_coords[y* num_tiles_x*4*2+x*4*2+ 2] = tileset_offset_x;
            tileset_tex_coords[y* num_tiles_x*4*2+x*4*2+3]  = tileset_offset_y;
 
            //bottom right
            tileset_tex_coords[y* num_tiles_x*4*2+x*4*2+4] = tileset_offset_x + tileset_inc_x;
            tileset_tex_coords[y* num_tiles_x*4*2+x*4*2+5] = tileset_offset_y + tileset_inc_y;
 
            //top right
            tileset_tex_coords[y* num_tiles_x*4*2+x*4*2+6] = tileset_offset_x + tileset_inc_x;
            tileset_tex_coords[y* num_tiles_x*4*2+x*4*2+7] = tileset_offset_y;
 
            tileset_offset_x += tileset_inc_x;
            //      for(int i =0 ; i < 8; i++)
            //        cout << " " <<        tileset_tex_coords[y* num_tiles_x*4*2+x*4*2+i];
            //      cout << endl;
        }
        tileset_offset_x = 0.0;
        tileset_offset_y += tileset_inc_y;
    }
}

/**
 * The function which generates the texture coordinates for the map
 * geometry, using the cached tile coordinates.
 */
void Map::generate_map_texcoords() {
#ifdef DEBUG
    printf("GENERATING MAP TEXTURE DATA...");
#endif
    //holds the map data
    //need 12 float for the 2D texture coordinates
    int num_floats = 12;
    int data_size = sizeof(GLfloat)*map_height*map_width*num_floats;
    map_tex_coords = new GLfloat[data_size]; 
    assert(map_tex_coords);

    int x, y;

    //generate the map data
    // get the tile set coordinates for the particular tile
    for(x = 0; x < map_width; x++) {
        for(y = 0; y < map_height; y++) {
            int curr_tile = world_data[x][y];

            GLfloat *tileset_ptr = &tileset_tex_coords[curr_tile*8];
            //bottom left
            map_tex_coords[x*map_height*num_floats + y*num_floats+0] = tileset_ptr[0];
            map_tex_coords[x*map_height*num_floats + y*num_floats+1] = tileset_ptr[1];

            //top left
            map_tex_coords[x*map_height*num_floats + y*num_floats+2] = tileset_ptr[2];
            map_tex_coords[x*map_height*num_floats + y*num_floats+3] = tileset_ptr[3];

            //bottom right
            map_tex_coords[x*map_height*num_floats + y*num_floats+4] = tileset_ptr[4];
            map_tex_coords[x*map_height*num_floats + y*num_floats+5] = tileset_ptr[5];

            //top left
            map_tex_coords[x*map_height*num_floats + y*num_floats+6] = tileset_ptr[2];
            map_tex_coords[x*map_height*num_floats + y*num_floats+7] = tileset_ptr[3];

            //top right
            map_tex_coords[x*map_height*num_floats + y*num_floats+8] = tileset_ptr[6];
            map_tex_coords[x*map_height*num_floats + y*num_floats+9] = tileset_ptr[7];
            
            //bottom right
            map_tex_coords[x*map_height*num_floats + y*num_floats+10] = tileset_ptr[4];
            map_tex_coords[x*map_height*num_floats + y*num_floats+11] = tileset_ptr[5];
        }
    }

    //Set this data in the renderable component
    renderable_component.set_texture_data(map_tex_coords, data_size, false);
    renderable_component.set_num_vertices_render(6*map_width*map_height);
}

/*
 * The function which generates the map geometry so that it can be
 * rendered to the screen
 */
void Map::generate_map_coords() {
#ifdef DEBUG
    printf("GENERATING MAP DATA...");
#endif
    //holds the map data
    //need 18 floats for each coordinate as these hold 3D coordinates
    int num_floats = 18;
    int data_size = sizeof(GLfloat)*map_height*map_width*num_floats;
    map_data = new GLfloat[data_size]; 
    assert(map_data);
    float scale = TILESET_ELEMENT_SIZE * GLOBAL_SCALE;
    //generate the map data
    ///
    /// Vertex winding order:
    /// 1, 3   4
    ///  * --- * 
    ///  |     |
    ///  |     | 
    ///  * --- *
    /// 0       2,5
    ///
    int x, y;
    for(x = 0; x < map_width; x++) {
        for(y = 0; y < map_height; y++) {
            //generate one tile's worth of data

            //bottom left
            map_data[x*map_height*num_floats + y*num_floats+ 0] = scale * float(x);
            map_data[x*map_height*num_floats + y*num_floats+ 1] = scale * float(y);
            map_data[x*map_height*num_floats + y*num_floats+ 2] = 0;
             
            //top left
            map_data[x*map_height*num_floats + y*num_floats+ 3] = scale * float(x);
            map_data[x*map_height*num_floats + y*num_floats+ 4] = scale * float(y + 1);
            map_data[x*map_height*num_floats + y*num_floats+ 5] = 0;

            //bottom right
            map_data[x*map_height*num_floats + y*num_floats+ 6] = scale * float(x + 1);
            map_data[x*map_height*num_floats + y*num_floats+ 7] = scale * float(y);
            map_data[x*map_height*num_floats + y*num_floats+ 8] = 0;
            
            //top left
            map_data[x*map_height*num_floats + y*num_floats+ 9]  = scale * float(x);
            map_data[x*map_height*num_floats + y*num_floats+10] = scale * float(y+1);
            map_data[x*map_height*num_floats + y*num_floats+11] = 0;
        
            //top right
            map_data[x*map_height*num_floats + y*num_floats+12] = scale * float(x+1);
            map_data[x*map_height*num_floats + y*num_floats+13] = scale * float(y+1);
            map_data[x*map_height*num_floats + y*num_floats+14] = 0;

            //bottom right
            map_data[x*map_height*num_floats + y*num_floats+15] = scale * float(x+1);
            map_data[x*map_height*num_floats + y*num_floats+16] = scale * float(y);
            map_data[x*map_height*num_floats + y*num_floats+17] = 0;
        }
    }

#ifdef DEBUG
    printf("DONE.");
#endif

   //Set this data in the renderable component
    renderable_component.set_vertex_data(map_data, data_size, false);
}


/** 
 * This function loads the required texture images
 */ 
void Map::load_tex_images() {
    FILE *tex_file1, *tex_file2 = nullptr;
    size_t bytes_read = 0;
    size_t image_sz_1 = IMAGE1_SIZE_WIDTH*IMAGE1_SIZE_HEIGHT*IMAGE1_NUM_COMPONENTS;
    size_t image_sz_2 = IMAGE2_SIZE_WIDTH*IMAGE2_SIZE_HEIGHT*IMAGE2_NUM_COMPONENTS;
    tex_buf[0] = new char[image_sz_1];

    tex_file1 = fopen(PATH "../resources/basictiles_2.raw", "rb");
    if(tex_file1 == nullptr) {
      std::cerr << "ERROR: Couldn't load textures" << std::endl;
    }

    if (tex_file1 && tex_buf[0]) {
        bytes_read = fread(tex_buf[0], 1, image_sz_1, tex_file1);
        assert(bytes_read == image_sz_1);  // some problem with file?
        fclose(tex_file1);
    }

    tex_buf[1] = new char[image_sz_2];

    tex_file2 = fopen(PATH "../resources/characters_1.raw", "rb");
    if(tex_file2 == nullptr) {
      std::cerr << "ERROR: Couldn't load textures" << std::endl;
    }

    if (tex_file2 && tex_buf[1]) {
        bytes_read = fread(tex_buf[1], 1, image_sz_2, tex_file2);
        assert(bytes_read == image_sz_2);  // some problem with file?
        fclose(tex_file2);
    }
}

/** 
 * This function performs the required Opengl initializatin for the textures.
 */ 
void Map::init_textures() {
    //  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    load_tex_images();
    glGenTextures(2, texture_ids);
    glBindTexture(GL_TEXTURE_2D, texture_ids[0]);
  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE1_SIZE_WIDTH, IMAGE1_SIZE_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, tex_buf[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture_ids[1]);
  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE2_SIZE_WIDTH, IMAGE2_SIZE_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, tex_buf[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);

}

/**
 * This function initialises the shader, creating and loading them.
 */ 
bool Map::init_shaders() {
#ifdef USE_GLES
  //read in the shaders
  std::ifstream vertex_shader_src("vert_shader.glesv");
  std::ifstream fragment_shader_src("frag_shader.glesf");
#endif
#ifdef USE_GL
  //read in the shaders
  std::ifstream vertex_shader_src("vert_shader.glv");
  std::ifstream fragment_shader_src("frag_shader.glf");
#endif

  if (!vertex_shader_src.good()){
    std::cerr << "Failed to load vertex shader" << std::endl;
    return false;
  }
    
  if (!fragment_shader_src.good()) {
    std::cerr << "Failed to load fragment shader" << std::endl;
    return false;
  }

  std::string vert_src, frag_src, line;
  while (getline(vertex_shader_src, line)) {
    vert_src += line + "\n";
  }

  while (getline(fragment_shader_src, line)) {
    frag_src += line + "\n";
  }

  Shader* shader = new Shader(vert_src, frag_src);

  if (!shader->is_loaded()) {
    delete shader;
    shader = NULL;
    std::cerr << "Failed to create the shader" << std::endl;
    return false;
  }

  return true;

}


/**
 * The function used to update elements on the map.
 */
void Map::update_map(float dt) {
}

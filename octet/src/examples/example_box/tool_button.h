#pragma once
namespace octet {
  class tool_button {
    private:
      bitmap_font font;
      GLuint font_texture;
      texture_shader texture_shader_;

      mat4t modelToWorld;
      GLuint modelToProjectionIndex_;

      float halfWidth;
      float halfHeight;
      float xPos, yPos;
      float rot_angle;

      std::vector<float> verts;
      GLuint vertex_buffer;
      shader b_shader;

      void(*clicked)();

      bool enabled;

    public:
      tool_button(){}

      void init(float x, float y, float w, float h, bitmap_font bm_font, void(*callback)()) {

        // Covert from pixels to ratio


        xPos = x;
        yPos = y;

        modelToWorld.loadIdentity();
        modelToWorld.translate(x, y, 0);
        rot_angle = 0;
        halfWidth = w * 0.5f;
        halfHeight = h * 0.5f;
        enabled = true;

        //font = bitmap_font(512, 256, "assets/big.fnt");
        font = bm_font;
        texture_shader_.init();
        font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

        verts.push_back(-halfWidth);
        verts.push_back(-halfHeight);
        verts.push_back(0.0f);
        verts.push_back(halfWidth);
        verts.push_back(-halfHeight);
        verts.push_back(0.0f);
        verts.push_back(halfWidth);
        verts.push_back(halfHeight);
        verts.push_back(0.0f);
        verts.push_back(-halfWidth);
        verts.push_back(-halfHeight);
        verts.push_back(0.0f);
        verts.push_back(halfWidth);
        verts.push_back(halfHeight);
        verts.push_back(0.0f);
        verts.push_back(-halfWidth);
        verts.push_back(halfHeight);
        verts.push_back(0.0f);

        glGenBuffers(1, &vertex_buffer); // Sets up our vertex array buffer for rendering
        //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        b_shader.init(RaceUtils::load_file("shaders/leaf.vert").c_str(), RaceUtils::load_file("shaders/leaf.frag").c_str()); // loads, compiles and links our shader programs
        modelToProjectionIndex_ = glGetUniformLocation(b_shader.get_program(), "modelToProjection");

        clicked = callback;
      }

      bool isClicked(int mouseX, int mouseY) {
        int mx = mouseX * 2 / 1200 - 1;
        int my = mouseY * 2 / 900 - 1;
        /*if (xPos < mx && mx < (xPos + width)) {
          if (yPos < my && my < (yPos + height)) {
            return true;
          }
        }*/
        return false;
      }

      void click() {
        clicked();
      }

      bool &is_enabled() {
        return enabled;
      }

      void render(mat4t &cameraToWorld) {
        click();
        // invisible sprite... used for gameplay.
        if (!is_enabled()) return;

        // build a projection matrix: model -> world -> camera -> projection
        // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
        //mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
        //----------------------
        mat4t worldToCamera;
        cameraToWorld.invertQuick(worldToCamera);

        // build a projection matrix to add perspective
        mat4t cameraToProjection;
        cameraToProjection.loadIdentity();
        cameraToProjection.ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.1f, 1000.0f);
        cameraToProjection.transpose4x4();

        mat4t modelToProjection = modelToWorld * worldToCamera * cameraToProjection;


        glUseProgram(b_shader.get_program());
        glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), &verts[0], GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(attribute_pos);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        
        glDrawArrays(GL_TRIANGLES, 0, verts.size() / 3);
        glDisableVertexAttribArray(attribute_pos);

        //TODO:: FIX draw text bug. Sussiption with the hash map class. get_value returning null entries.   draw_text(cameraToWorld, 1, "123");
        //char score_text[64];
        //sprintf(score_text, "test");
        //draw_text(cameraToWorld, 1.0f / 256, score_text);
      }


      void draw_text(mat4t &cameraToWorld, float scale, const char *text) {    
        mat4t textToWorld;
        textToWorld.loadIdentity();
        textToWorld.translate(xPos, yPos, 0);
        textToWorld.scale(scale, scale, 1);
        mat4t modelToProjection = mat4t::build_projection_matrix(textToWorld, cameraToWorld);

        /*mat4t tmp;
        glLoadIdentity();
        glTranslatef(x, y, 0);
        glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
        glScalef(scale, scale, 1);
        glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

        enum { max_quads = 32 };
        bitmap_font::vertex vertices[max_quads * 4];
        uint32_t indices[max_quads * 6];
        aabb bb(vec3(0, 0, 0), vec3(512, 512, 0));

        unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font_texture);

        texture_shader_.render(modelToProjection, 0);

        glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x);
        glEnableVertexAttribArray(attribute_pos);
        glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u);
        glEnableVertexAttribArray(attribute_uv);

        glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
      }


      // move the object
      void translate(float x, float y) {
        modelToWorld.translate(x, y, 0);
      }

      void rotate(float angle) {
        rot_angle += angle;
        modelToWorld.rotateZ(angle);
      }

      void set_rotation(float angle) {
        rotate(angle - rot_angle);
      }

      void get_position(const mat4t &cameraToWorld, float &x, float &y) {
        mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
        x = (float)modelToProjection[3][0];
        y = (float)modelToProjection[3][1];
      }

      void set_position(const float &x, const float &y) {
        modelToWorld[3][0] = x;
        modelToWorld[3][1] = y;
      }

      // position the object relative to another.
      void set_relative(tool_button &rhs, float x, float y) {
        modelToWorld = rhs.modelToWorld;
        modelToWorld.translate(x, y, 0);
      }

      // return true if this sprite collides with another.
      // note the "const"s which say we do not modify either sprite
      bool collides_with(const tool_button &rhs) const {
        float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
        float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

        // both distances have to be under the sum of the halfwidths
        // for a collision
        return
          (fabsf(dx) < halfWidth + rhs.halfWidth) &&
          (fabsf(dy) < halfHeight + rhs.halfHeight)
          ;
      }

      bool is_above(const tool_button &rhs, float margin) const {
        float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];

        return
          (fabsf(dx) < halfWidth + margin)
          ;
      }

  };
}
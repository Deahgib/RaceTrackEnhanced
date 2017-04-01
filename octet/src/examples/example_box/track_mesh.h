#pragma once

namespace octet {
  /// Scene containing a box with octet.
  class track_mesh {
  private:

    mat4t modelToWorld;
    GLuint modelToProjectionIndex_;

    float halfWidth;
    float halfHeight;

    float xPos, yPos;

    float rot_angle;

    bool enabled;

    GLuint vertex_buffer;
    shader road_shader;

  public:
    std::vector<float> vertBuff;
    std::vector<int> faceBuff;
    std::vector<vec3> debugBezBuff; // Used to show the actual bezier path with debug lines

    track_mesh() {}

    void init(float x, float y, float w, float h) {
      xPos = x;
      yPos = y;

      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      modelToWorld.scale(0.5f, 0.5f, 0.5f);

      rot_angle = 0;
      halfWidth = w * 0.5f;
      halfHeight = h * 0.5f;
      enabled = true;

      glGenBuffers(1, &vertex_buffer); // Sets up our vertex array buffer for rendering
      road_shader.init(RaceUtils::load_file("shaders/road.vert").c_str(), RaceUtils::load_file("shaders/road.frag").c_str()); // loads, compiles and links our shader programs
      modelToProjectionIndex_ = glGetUniformLocation(road_shader.get_program(), "modelToProjection");
    }


    void refresh_track(std::vector<float> newVertBuff, std::vector<int> newFaceBuff, std::vector<vec3> newDebugBezBuff) {
      debugBezBuff = newDebugBezBuff;
      vertBuff = newVertBuff;
      faceBuff = newFaceBuff;
    }


    void render(mat4t &cameraToWorld) {

      if (!is_enabled()) return;
       

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

      glUseProgram(road_shader.get_program());
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());

      // Grass green colour
      // openGL takes in an array of floats. Every 3 floats represents one vertex. 
      // Bellow is code telling opengl what float vertex data to use.
      // openGL reads the raw bytes in memory, so we need to tell it how many bytes per value (in this case float 4 bytes) 
      // and we also need to tell it how many values per vertex (in this case 3 for x, y and z)
      // We then tell openGL what shader program to use to render the mesh 
      // and we specify the render mode, here, GL_TRIANGLE_STRIP tells opengl to make the vertex data connect up into a mesh like this:
      //  The numbers represent the vertices, each vertex is three floats wide (z,y,z)
      //
      //   0-----2-----4
      //   |    /|    /|
      //   |   / |   / |
      //   |  /  |  /  |
      //   | /   | /   |
      //   |/    |/    |
      //   1-----3-----5

      //std::vector<float> leftTrackVertBuff = vertBuff;
      //std::vector<float> rightTrackVertBuff = vertBuff;
      //for (int i = 0; i < leftTrackVertBuff.size(); i += 3) {
      //  leftTrackVertBuff[i] = 0.5f * leftTrackVertBuff[i] - 0.5f;
      //  leftTrackVertBuff[i + 1] = 0.66666f * leftTrackVertBuff[i + 1] - 0.33333f;
      //}


      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, vertBuff.size() * sizeof(GLfloat), &vertBuff[0], GL_DYNAMIC_DRAW);


      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
      //glUseProgram(road_shader.get_program());
      glDrawArrays(GL_TRIANGLE_STRIP, 0, vertBuff.size() / 3);
      glDisableVertexAttribArray(attribute_pos);


      //for (int i = 0; i < rightTrackVertBuff.size(); i += 3) {
      //  rightTrackVertBuff[i + 1] = cos(90)*rightTrackVertBuff[i + 1] - sin(90)*rightTrackVertBuff[i + 2];
      //  rightTrackVertBuff[i + 2] = sin(90)*rightTrackVertBuff[i + 1] + cos(90)*rightTrackVertBuff[i + 2];

      //  rightTrackVertBuff[i] = 0.5f * rightTrackVertBuff[i] + 0.5f;
      //  rightTrackVertBuff[i + 2] = 0.66666f * rightTrackVertBuff[i + 1] - 0.33333f;
      //}
      //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      //glBufferData(GL_ARRAY_BUFFER, rightTrackVertBuff.size() * sizeof(GLfloat), &rightTrackVertBuff[0], GL_DYNAMIC_DRAW);
      //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
      //glEnableVertexAttribArray(attribute_pos);
      //glUseProgram(road_shader.get_program());
      //glDrawArrays(GL_TRIANGLE_STRIP, 0, rightTrackVertBuff.size() / 3);
      //glBindVertexArray(attribute_pos);

    }
    
    void draw_debug(std::vector<vec3> waypoints) {
      /* https://en.wikibooks.org/wiki/OpenGL_Programming/GLStart/Tut3 */

      //glClearColor(0.3f, 0.67f, 0.28f, 1.0f);

      // Draw the start and end waypoints in yellow
      glUseProgram(0);
      glColor3f(1.0f, 1.0f, 0.0f); //yellow colour
      glPointSize(5.0f);//set point size to 10 pixels
      glBegin(GL_POINTS); //starts drawing of points
      glVertex3f(waypoints[0][0], waypoints[0][1], waypoints[0][2]);
      glVertex3f(waypoints[waypoints.size() - 1][0], waypoints[waypoints.size() - 1][1], waypoints[waypoints.size() - 1][2]);
      glEnd();

      // Draw the waypoints
      glColor3f(1.0f, 0.0f, 0.0f); //red colour

      glBegin(GL_POINTS); //starts drawing of points
      for (vec3 &point : waypoints) {
        glVertex3f(point[0], point[1], point[2]);
      }
      glEnd();//end drawing of points

              // Draw the bezzier line.
      glColor3f(0.0f, 1.0f, 0.0f); //green colour
      glBegin(GL_LINE_STRIP); //starts drawing of line_strip
      for (int i = 0; i < debugBezBuff.size() - 1; i++) {
        glVertex3f(debugBezBuff[i][0], debugBezBuff[i][1], debugBezBuff[i][2]);
        glVertex3f(debugBezBuff[i + 1][0], debugBezBuff[i + 1][1], debugBezBuff[i + 1][2]);
      }
      glEnd();//end drawing of Line_strip


      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, vertBuff.size() * sizeof(GLfloat), &vertBuff[0], GL_DYNAMIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
      glEnableVertexAttribArray(attribute_pos);
      glUseProgram(road_shader.get_program());
      glDrawArrays(GL_LINE_STRIP, 0, vertBuff.size() / 3);
      glBindVertexArray(attribute_pos);

    }

    bool &is_enabled() {
      return enabled;
    }

    // move the object
    void translate(float x, float y) {
      modelToWorld.translate(x, y, 0);
    }

    void rotate(float angle) {
      rot_angle += angle;
      modelToWorld.rotateZ(angle);
    }

    void rotate(float angle, float X, float Y, float Z) {
      if(Z != 0) rot_angle += angle;
      modelToWorld.rotate(angle, X, Y, Z);
    }

    void scale(float scaleX, float scaleY, float scaleZ) {
      modelToWorld.scale(scaleX, scaleY, scaleZ);
    }

  };
}
////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//


#include <stdlib.h>
#include <ctime>
#include <functional>
#include "points_generator.h"
#include "perlin.h"
#include "RaceUtils.h"
#include "track_mesh.h"
#include "tool_button.h"


namespace octet {
  /// Scene containing a box with octet.
  class example_box : public app {
  private:

    bitmap_font font;

    mat4t cameraToWorld;
    track_mesh track;
    track_mesh side_track;
    perlin perlin_noise;
    points_generator pg;
    std::vector<vec3> waypoints;

    enum curve_mode {
      QUADRATIC_BEZIER = 0,
      CUBIC_BEZIER,
      CATMULL_ROM
    };
    curve_mode current_curve;
    curve_mode prev_curve;

    bool debug_mode = false;

    //// scene for drawing box
    //ref<visual_scene> app_scene;


    float TRACK_WIDTH;
    float DETAIL_STEP;
    float height_scale;
    int track_length;
    int curve_step;
    bool heightMode2D = true;
    bool smooth_track = false;

    unsigned int seed;


    void clear_curve() {
      seed = std::time(nullptr);
      //seed = 10000;
      TRACK_WIDTH = 0.02f;
      DETAIL_STEP = 0.01f;
      height_scale = 0.5f;
      track_length = 8;
    }

    //TEMPORARY DEBUG VAR
    double minSimilarity =0;

    void refresh_curve() {
      switch (current_curve) {
      case QUADRATIC_BEZIER:
        curve_step = 2;
        if (track_length < 3) return;
        break;
      case CUBIC_BEZIER:
        curve_step = 3;
        if (track_length < 4) return;
        break;
      case CATMULL_ROM:
        curve_step = 1;
        if (track_length < 4) return;
        break;
      }

      perlin_noise = perlin(seed);

      // create points for curves
      int num_points = curve_step * track_length + 1;
      waypoints = std::vector<vec3>();
      waypoints = pg.generate_radial_points(num_points, seed);


      std::vector<vec3> debugBezBuff = std::vector<vec3>();
      std::vector<float> vertBuff = std::vector<float>();
      std::vector<int> faceBuff = std::vector<int>();

      int vertPair = 0;

      // This is to avoid an infinite loop when adding points...
      // ...to the vector during the loop.
      int track_size = waypoints.size();

      for (int i = 0; i < track_size; i += curve_step) {
        // Connecting the start and the end points of the track...
        // ...by generating points.
        if ((i == track_size - 1) 
          && (current_curve == QUADRATIC_BEZIER || CUBIC_BEZIER))
        {
          vec3 v_point = (waypoints[i] + waypoints[0]) / 2;
          waypoints.push_back(v_point);
          if (current_curve == CUBIC_BEZIER) {
            v_point = (waypoints[i+1] + waypoints[0]) / 2;
            waypoints.push_back(v_point);
          }
          v_point = waypoints[0];
          waypoints.push_back(v_point);
        }

        // Getting rid of end points when changing to CATMULL_ROM
        if ((i == track_size - 1) 
          && (prev_curve == QUADRATIC_BEZIER || CUBIC_BEZIER) 
          && (current_curve == CATMULL_ROM))
        {  
          waypoints.pop_back();
          waypoints.pop_back();
          if (prev_curve == CUBIC_BEZIER) {
            waypoints.pop_back();
          }
        }

        for (float t = 0.0f; t <= 1.0f; t += DETAIL_STEP) {
          vec3 pos = get_bezier_point(t, i);
          vec3 segment_pos = get_bezier_point(t + DETAIL_STEP * 0.01f, i);
          vec3 tan = segment_pos - pos;
          tan = tan.normalize();
          vec3 norm;
          if (!smooth_track) {
            norm = tan.cross(vec3(0, 0, 1)); // Get normal from tangent.
          }
          else 
          {
            vec3 next_pos = get_bezier_point(t + DETAIL_STEP*1, i);
            vec3 next_segment_pos = get_bezier_point(t + DETAIL_STEP*1 + DETAIL_STEP * 0.01f, i);
            vec3 next_tan = next_segment_pos - next_pos;
            next_tan = next_tan.normalize();

            double similarity = (1 - tan.dot(next_tan)) * 1 / DETAIL_STEP;
            if (similarity > 0.5f) similarity = 0.5f;
            if (similarity > minSimilarity) {
              printf("Found new min: %lf\n", similarity);
              minSimilarity = similarity;
            }
            // Angle of the next_tan vector to the world Y axis for later rotation
            float angle = vec3(0, 1, 0).dot(next_tan) / next_tan.length();

            // Work out if the next tan is veering left or right.
            float direction = tan.cross(vec3(0, 0, 1)).dot(next_tan);
            direction = direction / abs(direction);

            mat4t rotation;
            rotation.loadIdentity();
            rotation.rotate(90.0f * similarity * direction /** abs(ntan[1])*/, 0, 1, 0);
            rotation.rotate(angle, 0, 0, 1);
            vec3 upVec = vec3(0, 0, 1);
            upVec = upVec * rotation;
          
            norm = tan.cross(upVec);
          }

          double n;
          if(heightMode2D)
            n = (float)perlin_noise.noise((double)pos[0], (double)pos[1], 0.0) * height_scale;
          else
            n = (double)(perlin_noise.noise((double)(t+(float)i), (double)(t + (float)i), 0.0) * height_scale);


          norm = norm.normalize() * TRACK_WIDTH * 0.5f; // Create track radius

          vec3 p1 = pos - norm; // Calculate border vertex locations
          vec3 p2 = pos + norm;

          vertBuff.push_back(p1[0]); // Add vertex data (3 Floats (x, y and y)) to the buffer
          vertBuff.push_back(p1[1]); // The buffer is used by opengl to render the triangles
          vertBuff.push_back(p1[2]+n); // Use the perlin height at the center of the track for this point along the track.
          vertBuff.push_back(p2[0]);
          vertBuff.push_back(p2[1]);
          vertBuff.push_back(p2[2] + n);

          if (vertPair > 0) {
            faceBuff.push_back(vertPair * 2 - 2);
            faceBuff.push_back(vertPair * 2 - 1);
            faceBuff.push_back(vertPair * 2);

            faceBuff.push_back(vertPair * 2 - 1);
            faceBuff.push_back(vertPair * 2 + 1);
            faceBuff.push_back(vertPair * 2);
          }
          vertPair++;

          debugBezBuff.push_back(pos);
        }
      }
      track.refresh_track(vertBuff, faceBuff, debugBezBuff);
      side_track.refresh_track(vertBuff, faceBuff, debugBezBuff);

      
      printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
      printf("RACE TRACK\nF5: Randomise New Track\nF6: Save track\nF1: Quad Bezier Mode\nF2: Cubic Bezier Mode\nF3: Catmull Rom Mode\nF4: Toggle Height Mode (Linear & 2D)\nUp & Down: Track Length\nRight & Left: Track Width\nF7 & F8: Height Amplitude\nF9 & F10: Mesh detail\n_____________________\n");
      printf("Seed: %d\n_____________________\nTrack width: %f\nMesh Detail: %f\nHeight Scale: %f\nTrack Length: %d\n_____________________\n", seed, TRACK_WIDTH, DETAIL_STEP, height_scale, track_length);
      printf("Mesh with %d vertices\n", (int) vertBuff.size() / 3);
      printf("%d total faces\n", (int) faceBuff.size() / 3);
    }

    vec3 get_bezier_point(float t, int iter) {
      vec3 point(0, 0, 0);

      // Glitch fix
      if (t > 1.0f) {
        //printf("Tangent calculation glitching over into next points group\n"); 
        t = t - 1.0f;
        iter += curve_step;
      }

      //sorted some variables 
      float tt = t * t;
      float ttt = t * t * t;
      float u = (1 - t);
      float uu = u * u;
      float uuu = u * u * u;

      // Repoints to the front of the waypoints list if iter + n exceeds vector bounds
      int idx = iter;
      int idx1 = iter + 1;
      int idx2 = iter + 2;
      int idx3 = iter + 3;
      if ((waypoints.size() <= (iter))) {
        idx = 0;
        idx1 = 1;
        idx2 = 2;
        idx3 = 3;
      }
      else if ((waypoints.size() <= (iter + 1))) {
        idx1 = 0;
        idx2 = 1;
        idx3 = 2;
      }
      else if ((waypoints.size() <= (iter + 2))) {
        idx2 = 0;
        idx3 = 1;
      }
      else if ((waypoints.size() <= (iter + 3))) {
        idx3 = 0;
      }

      switch (current_curve) {

      case QUADRATIC_BEZIER:

        point[0] = uu * waypoints[idx][0] + 2 * u * t * waypoints[idx1][0] + tt * waypoints[idx2][0];
        point[1] = uu * waypoints[idx][1] + 2 * u * t * waypoints[idx1][1] + tt * waypoints[idx2][1];
        
        break;

      case CUBIC_BEZIER:

        point[0] = uuu * waypoints[idx][0] + 3 * uu * t * waypoints[idx1][0] + 3 * u * tt* waypoints[idx2][0] + ttt * waypoints[idx3][0];
        point[1] = uuu * waypoints[idx][1] + 3 * uu * t * waypoints[idx1][1] + 3 * u * tt* waypoints[idx2][1] + ttt * waypoints[idx3][1];

        break;

      case CATMULL_ROM:

        point[0] = 0.5f * ((-t) * uu * waypoints[idx][0] + (2 - 5 * tt + 3 * ttt) * waypoints[idx1][0] + t * (1 + 4 * t - 3 * tt) * waypoints[idx2][0] - tt * u * waypoints[idx3][0]);
        point[1] = 0.5f * ((-t) * uu * waypoints[idx][1] + (2 - 5 * tt + 3 * ttt) * waypoints[idx1][1] + t * (1 + 4 * t - 3 * tt) * waypoints[idx2][1] - tt * u * waypoints[idx3][1]);

        break;
      }

      return point;
    }

  public:

    int window_w = 1200;
    int window_h = 900;

    /// this is called when we construct the class before everything is initialised.
    example_box(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
    }

    //static example_box controller;
    //static void testFunc() {
    //  printf("Button Pressed");
    //  controller->current_curve = CUBIC_BEZIER;
    //  controller->refresh_curve();
    //}


    tool_button b;

    /// this is called once OpenGL is initialized
    void app_init() {

      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 3);
      //cameraToWorld.rotateX(180);
      //cameraToWorld.rotateY(180);

      track = track_mesh();
      track.init(-2, -1, 1, 1);
      track.scale(4, 4, 4);

      side_track = track_mesh();
      side_track.init(2, -1.5, 1, 1);
      side_track.scale(4, 4, 4);
      side_track.rotate(-90, 1, 0, 0);

      perlin_noise = perlin();
      pg = points_generator();

      current_curve = CATMULL_ROM;
      track_length = 10;

      clear_curve();
      refresh_curve();


      /*std::function<void(example_box* parent)> fn1 = testFunc;
      testFunc(this);*/
      //b = tool_button();
      //b.init(0, 2, 7.9f, 1);

    }


    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      

      //int mx, my = 0;;
      //get_mouse_pos(mx, my);
      //if (b.isClicked(mx, my)) {
      //  b.click();
      //}

      if (is_key_going_up(key_f5)) {
        clear_curve();
        refresh_curve();
      }

      if (is_key_going_down(key_f6)) {
        float scale_mult = 20.0f / TRACK_WIDTH;
        std::vector<float> vertexData = track.vertBuff;
        for (int i = 0; i < vertexData.size(); i++) {
          vertexData[i] *= scale_mult;
        }
        RaceUtils::file_create("race-track.ply", vertexData, track.faceBuff);
      }

      if (is_key_going_up(key_f1)) {
        current_curve = QUADRATIC_BEZIER;
        refresh_curve();
      }
      if (is_key_going_up(key_f2)) {
        current_curve = CUBIC_BEZIER;
        refresh_curve();
      }
      if (is_key_going_up(key_f3)) {
        prev_curve = current_curve;
        current_curve = CATMULL_ROM;
        refresh_curve();
      }

      if (is_key_going_up(key_space)) {
        debug_mode = !debug_mode;
      }

      if (is_key_going_up(key_up)) {
        track_length++;
        refresh_curve();
      }
      if (is_key_going_up(key_down)) {
        track_length--;
        refresh_curve();
      }

      if (is_key_going_up(key_right)) {
        TRACK_WIDTH += 0.02f;
        refresh_curve();
      }
      if (is_key_going_up(key_left)) {
        TRACK_WIDTH -= 0.02f;
        refresh_curve();
      }

      if (is_key_going_up(key_f8)) {
        if (height_scale < 1.0f) {
          height_scale += 0.1f;
          refresh_curve();
        }
      }
      if (is_key_going_up(key_f7)) {
        if(height_scale > 0.0f){
          height_scale -= 0.1f;
          refresh_curve();
        }
      }

      if (is_key_going_up(key_f10)) {
        if (DETAIL_STEP > 0.01f) {
          DETAIL_STEP -= 0.01f;
          refresh_curve();
        }
      }
      if (is_key_going_up(key_f9)) {
        if (DETAIL_STEP < 0.1f) {
          DETAIL_STEP += 0.01f;
          refresh_curve();
        }
      }
      if (is_key_going_up(key_f4)) {
        heightMode2D = !heightMode2D;
        refresh_curve();
      }
      if (is_key_going_up(key_ctrl)) {
        smooth_track = !smooth_track;
        refresh_curve();
      }


      // RENDERING

      // Local values for viewport
      app_common::get_viewport_size(window_w, window_h);

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      glClearColor(0.3f, 0.67f, 0.28f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


      //b.render(cameraToWorld);
      side_track.rotate(1, 0, 0, 1);
      track.render(cameraToWorld);
      side_track.render(cameraToWorld);
      //if (debug_mode) track.draw_debug(waypoints);
      //else track.render(cameraToWorld);

      if (!debug_mode) {
        glUseProgram(0);
        glColor3f(0.0f, 0.0f, 0.0f); //yellow colour
        glBegin(GL_LINES); //starts drawing of points
        glVertex3f(-1, 0.33333f, 0);
        glVertex3f(1, 0.33333f, 0);
        glVertex3f(0, 0.33333f, 0);
        glVertex3f(0, -1, 0);
        glEnd();
      }
    }
  };
}

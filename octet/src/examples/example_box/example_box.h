﻿////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//


#include <stdlib.h>
#include <ctime>
#include "points_generator.h"
#include "perlin.h"
#include "RaceUtils.h"
#include "track_mesh.h"


namespace octet {
  /// Scene containing a box with octet.
  class example_box : public app {
  private:

    track_mesh track;
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

    bool debug_mode = true;

    //// scene for drawing box
    //ref<visual_scene> app_scene;


    float TRACK_WIDTH;
    float DETAIL_STEP;
    float height_scale;
    int track_length;
    int curve_step;

    unsigned int seed;


    void clear_curve() {
      seed = std::time(nullptr);
      seed = 10000;
      TRACK_WIDTH = 0.00f;
      DETAIL_STEP = 0.01f;
      height_scale = 0.5f;
      track_length = 80;
    }

    void refresh_curve() {
      switch (current_curve) {
      case QUADRATIC_BEZIER:
        curve_step = 2;
        break;
      case CUBIC_BEZIER:
        curve_step = 3;
        break;
      case CATMULL_ROM:
        curve_step = 1;
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
          vec3 norm = tan.cross(vec3(0, 0, 1)); // Get normal from tangent.

          double n = (float)perlin_noise.noise((double)pos[0], (double)pos[1], 0.0) * height_scale;

          norm = norm.normalize() * TRACK_WIDTH * 0.5f; // Create track radius

          vec3 p1 = pos - norm; // Calculate border vertex locations
          vec3 p2 = pos + norm;

          vertBuff.push_back(p1[0]); // Add vertex data (3 Floats (x, y and y)) to the buffer
          vertBuff.push_back(p1[1]); // The buffer is used by opengl to render the triangles
          vertBuff.push_back(n); // Use the perlin height at the center of the track for this point along the track.
          vertBuff.push_back(p2[0]);
          vertBuff.push_back(p2[1]);
          vertBuff.push_back(n);

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
      /*
          float TRACK_WIDTH = 0.1f;
    float DETAIL_STEP = 0.01f;
    float height_scale = 0.5f;
    int track_length = 10;
    int curve_step;
      */
      printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
      printf("RACE TRACK\n_____________________\nTrack width: %f\nMesh Detail: %f\nHeight Scale: %f\nTrack Length: %d\n_____________________\n", TRACK_WIDTH, DETAIL_STEP, height_scale, track_length);
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
    /// this is called when we construct the class before everything is initialised.
    example_box(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      track = track_mesh();
      track.init();
      perlin_noise = perlin();
      pg = points_generator();

      current_curve = CATMULL_ROM;
      track_length = 10;

      clear_curve();
      refresh_curve();
    }

    


    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

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
        height_scale += 0.1f;
        refresh_curve();
      }
      if (is_key_going_up(key_f7)) {
        height_scale -= 0.1f;
        refresh_curve();
      }

      if (is_key_going_up(key_f10)) {
        DETAIL_STEP -= 0.01f;
        refresh_curve();
      }
      if (is_key_going_up(key_f9)) {
        DETAIL_STEP += 0.01f;
        refresh_curve();
      }

      if (debug_mode) track.draw_debug(waypoints);
      else track.render();

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

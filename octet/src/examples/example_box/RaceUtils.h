#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>


namespace octet {
  /// Scene containing a box with octet.
  class RaceUtils {
  public:

    // Used to load shader files into a string varaible
    static std::string load_file(const char* file_name) {
      std::ifstream is(file_name);
      if (is.bad() || !is.is_open()) return nullptr;
      char buffer[2048];
      // loop over lines
      std::string out;
      while (!is.eof()) {
        is.getline(buffer, sizeof(buffer));
        out += buffer;
        out += "\n";
      }
      //printf("%s", out.c_str());
      return out;
    }


    static void file_create(const char* file_name, std::vector<float> vertexData, std::vector<int> faceBuff) {
      std::ofstream raceTrack;
      raceTrack.open(file_name);

      raceTrack << "ply\n";
      raceTrack << "format ascii 1.0\n";
      raceTrack << "element vertex " << (int)vertexData.size() / 3 << "\n";
      raceTrack << "property float x\n";
      raceTrack << "property float y\n";
      raceTrack << "property float z\n";
      raceTrack << "element face " << (int)faceBuff.size() / 3 << "\n";
      raceTrack << "property list uint8 int32 vertex_indices\n";
      raceTrack << "end_header\n";

      //vertices
      for (int i = 0; i < vertexData.size(); i++) {
        raceTrack << vertexData[i] << " ";
        if ((i + 1) % 3 == 0) {
          raceTrack << "\n";
        }
      }

      //faces
      for (int j = 0; j < faceBuff.size(); j++) {

        if ((j) % 3 == 0) {
          raceTrack << "3 ";
        }

        raceTrack << faceBuff[j] << " ";
        if ((j + 1) % 3 == 0) {
          raceTrack << "\n";
        }
      }
      raceTrack.close();


    }

  };
}
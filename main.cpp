#include "Detector.h"
#include "Image.h"

int main() {
  ShotBoundaryDetector detector("./dataset/ad.rgb", 480, 270);
  // ShotBoundaryDetector detector("./dataset/Videos/data_test1.rgb", 480, 270);
  detector.StartDetection();
  return 0;
}

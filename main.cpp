#include "Detector.h"
#include "Image.h"

int main() {
  ShotBoundaryDetector detector("ad.rgb", 480, 270);
  detector.StartDetection();
  return 0;
}

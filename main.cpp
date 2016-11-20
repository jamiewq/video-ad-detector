#include "Detector.h"
#include "Image.h"
#include <ctime>
int main() {
  ShotBoundaryDetector detector("./dataset/ad.rgb", 480, 270);
  // ShotBoundaryDetector detector("./dataset/Videos/data_test1.rgb", 480, 270);
  int start_s=clock();
	detector.StartDetection();
  int stop_s=clock();
  cout << "time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << endl;
  return 0;
}

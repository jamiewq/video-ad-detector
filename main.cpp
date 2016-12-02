#include "Detector.h"
#include <ctime>
int main() {

  // ShotBoundaryDetector detector("./dataset2/Ads/nfl_Ad_15s.rgb", 480, 270);
  // ShotBoundaryDetector detector("./dataset2/Ads/mcd_Ad_15s.rgb", 480, 270);
  // ShotBoundaryDetector detector("./dataset2/Videos/data_test2.rgb", 480, 270);
  // ShotBoundaryDetector detector("./dataset/Ads/Starbucks_Ad_15s.rgb", 480, 270);
  // ShotBoundaryDetector detector("./dataset/Ads/Subway_Ad_15s.rgb", 480, 270);
  ShotBoundaryDetector detector("./dataset/Videos/data_test1.rgb", 480, 270);

  detector.display_cut(true);
  detector.set_similar_threshold(10);
  int start_s = clock();
  detector.StartDetection();
  int stop_s=clock();
  cout << "time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << endl;

  vector<pair<long, long> > ads = detector.get_ad_list();
  return 0;
}

#include "Detector.h"
#include <ctime>
int main() {

  // ShotBoundaryDetector detector("./dataset/Videos/data_test1.rgb", 480, 270);
  ShotBoundaryDetector detector("./dataset/Ads/Subway_Ad_15s.rgb", 480, 270);
  // ShotBoundaryDetector detector("./dataset/Ads/Starbucks_Ad_15s.rgb", 480, 270);

  // detector.display_cut(true);
  detector.set_similar_threshold(6);
  detector.set_cut_threshold(10);
  detector.set_min_ad_length(10); // 3 seconds
  detector.set_max_ad_length(3600); // 2 min
  detector.set_max_ad_shot(600); // priori knowledge, boundary between ad and main shot. No worry to much about this, as we consider great changes first
  detector.set_max_delta(30); //  delta = length[i]/length[i-1] or length[i-1]/length[i] whichever >= 1
  int start_s = clock();
  detector.StartDetection();
  int stop_s = clock();
  cout << "time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << endl;

  vector<pair<long, long> > ads = detector.get_ad_list();
  return 0;
}

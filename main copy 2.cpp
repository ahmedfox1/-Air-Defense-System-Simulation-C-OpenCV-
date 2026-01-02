#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace cv;
using namespace std;

string getCurrentTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    stringstream ss;
    ss << setw(2) << setfill('0') << ltm->tm_hour << ":"
       << setw(2) << setfill('0') << ltm->tm_min << ":"
       << setw(2) << setfill('0') << ltm->tm_sec;
    return ss.str();
}

void drawBracket(Mat& img, int x, int y, int w, int h, Scalar color) {
    int len = w / 4;
    line(img, Point(x, y), Point(x + len, y), color, 2);
    line(img, Point(x, y), Point(x, y + len), color, 2);
    line(img, Point(x + w, y), Point(x + w - len, y), color, 2);
    line(img, Point(x + w, y), Point(x + w, y + len), color, 2);
    line(img, Point(x, y + h), Point(x + len, y + h), color, 2);
    line(img, Point(x, y + h), Point(x, y + h - len), color, 2);
    line(img, Point(x + w, y + h), Point(x + w - len, y + h), color, 2);
    line(img, Point(x + w, y + h), Point(x + w, y + h - len), color, 2);
}

// دالة لرسم الانفجار
void drawExplosion(Mat& img, Point center, int frameState) {
    Scalar color1(0, 255, 255); // أصفر
    Scalar color2(0, 0, 255);   // أحمر

    int radius = frameState * 15;

    // دائرة خارجية حمراء
    circle(img, center, radius, color2, 4);
    // دائرة داخلية صفراء (كرة النار)
    circle(img, center, radius / 2, color1, FILLED);

    // شظايا
    line(img, center, Point(center.x + radius + 10, center.y + radius + 10), color1, 2);
    line(img, center, Point(center.x - radius - 10, center.y - radius - 10), color1, 2);
    line(img, center, Point(center.x + radius + 10, center.y - radius - 10), color1, 2);
    line(img, center, Point(center.x - radius - 10, center.y + radius + 10), color1, 2);
}

int main() {
    VideoCapture cap(1); // تأكد من رقم الكاميرا
    if (!cap.isOpened()) return -1;

    Scalar cyan(255, 255, 0);
    Scalar red(0, 0, 255);
    Scalar green(0, 255, 0);
    Scalar white(200, 200, 200);
    Scalar yellow(0, 255, 255); // لون الصاروخ

    Scalar lowerBlue(100, 150, 0);
    Scalar upperBlue(140, 255, 255);

    Mat frame, hsv, mask, display;
    int radarSweep = 0;
    int counter = 0;

    // متغيرات الصاروخ
    bool missileActive = false;
    Point missilePos;
    bool explosionActive = false;
    int explosionTimer = 0;

    vector<string> logData;
    logData.push_back("SYS_INIT... OK");
    logData.push_back("RADAR_M... ACTIVE");
    logData.push_back("LINK_16... SECURE");

    namedWindow("EGY_ADS_V2", WINDOW_NORMAL);
    resizeWindow("EGY_ADS_V2", 1280, 720);

    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        flip(frame, frame, 1);
        resize(frame, frame, Size(1024, 600));

        display = frame.clone();

        // Darken background
        Mat overlay;
        display.copyTo(overlay);
        rectangle(overlay, Point(0,0), Point(display.cols, display.rows), Scalar(0,20,0), FILLED);
        addWeighted(overlay, 0.3, display, 0.7, 0, display);

        // Scanlines
        for(int y=0; y<display.rows; y+=4) {
            line(display, Point(0, y), Point(display.cols, y), Scalar(0,0,0), 1);
        }

        cvtColor(frame, hsv, COLOR_BGR2HSV);
        inRange(hsv, lowerBlue, upperBlue, mask);

        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        bool locked = false;
        Rect targetBox;
        Point center;

        if (!contours.empty()) {
            double maxArea = 0; int idx = -1;
            for (int i=0; i<contours.size(); i++) {
                double a = contourArea(contours[i]);
                if (a > maxArea) { maxArea = a; idx = i; }
            }
            if (idx != -1 && maxArea > 400) {
                locked = true;
                targetBox = boundingRect(contours[idx]);
                center = Point(targetBox.x + targetBox.width/2, targetBox.y + targetBox.height/2);
            }
        }

        // --- LEFT SIDE DATA ---
        int leftX = 20;
        putText(display, "UNIT: 777-AGR", Point(leftX, 40), FONT_HERSHEY_SIMPLEX, 0.6, cyan, 1);
        putText(display, "SEC: CAIRO_N", Point(leftX, 65), FONT_HERSHEY_SIMPLEX, 0.6, cyan, 1);
        putText(display, "LAT: 30.0444 N", Point(leftX, 90), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
        putText(display, "LON: 31.2357 E", Point(leftX, 110), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);

        rectangle(display, Point(leftX, 130), Point(leftX+150, 400), cyan, 1);
        putText(display, "SYSTEM LOG", Point(leftX+5, 145), FONT_HERSHEY_PLAIN, 1, cyan, 1);

        if (counter % 50 == 0) {
            stringstream ss; ss << "PING: " << rand()%90 + 10 << "ms";
            logData.push_back(ss.str());
            if (logData.size() > 10) logData.erase(logData.begin());
        }
        for(int i=0; i<logData.size(); i++) {
            putText(display, logData[i], Point(leftX+5, 170 + i*20), FONT_HERSHEY_PLAIN, 0.9, white, 1);
        }

        // --- RIGHT SIDE DATA ---
        int rightX = display.cols - 180;
        putText(display, "WIND: 12 KTS", Point(rightX, 40), FONT_HERSHEY_PLAIN, 1, cyan, 1);
        putText(display, "VIS: 10 KM", Point(rightX, 60), FONT_HERSHEY_PLAIN, 1, cyan, 1);
        putText(display, "TEMP: 34 C", Point(rightX, 80), FONT_HERSHEY_PLAIN, 1, cyan, 1);

        // Altitude Tape
        line(display, Point(rightX-20, 100), Point(rightX-20, 400), cyan, 2);
        for(int i=0; i<10; i++) {
            line(display, Point(rightX-20, 120 + i*30), Point(rightX-10, 120 + i*30), cyan, 1);
            putText(display, to_string(1000 - i*100), Point(rightX, 125 + i*30), FONT_HERSHEY_PLAIN, 0.8, white, 1);
        }
        int altY = 400 - (center.y * 300 / display.rows);
        if(!locked) altY = 250;
        line(display, Point(rightX-25, altY), Point(rightX-5, altY), red, 2);
        putText(display, "ALT", Point(rightX-45, altY+5), FONT_HERSHEY_PLAIN, 1, red, 1);

        // --- TOP BAR ---
        rectangle(display, Point(300, 10), Point(724, 50), Scalar(0, 50, 0), FILLED);
        line(display, Point(512, 10), Point(512, 60), red, 2);
        putText(display, getCurrentTime(), Point(display.cols/2 - 50, 80), FONT_HERSHEY_SIMPLEX, 0.6, cyan, 1);

        // --- BOTTOM BAR (WEAPONS) ---
        int botY = display.rows - 60;
        for(int i=0; i<4; i++) {
            Scalar wpnColor = cyan;
            string wpnText = "RDY";
            // لو الصاروخ انضرب، نغير لون أول مربع
            if (missileActive || explosionActive) {
                if (i==0) { wpnColor = red; wpnText = "EMPTY"; }
            }

            rectangle(display, Point(300 + i*110, botY), Point(400 + i*110, botY+40), wpnColor, 1);
            putText(display, "M-" + to_string(i+1), Point(310 + i*110, botY+25), FONT_HERSHEY_PLAIN, 1, white, 1);
            putText(display, wpnText, Point(360 + i*110, botY+25), FONT_HERSHEY_PLAIN, 1, (i==0 && (missileActive||explosionActive)) ? red : green, 1);
        }

        // --- RADAR CIRCLE ---
        int radX = 100, radY = display.rows - 100;
        circle(display, Point(radX, radY), 70, Scalar(0,100,0), 1);
        radarSweep = (radarSweep + 5) % 360;
        float ang = radarSweep * CV_PI / 180;
        line(display, Point(radX, radY), Point(radX + 70*cos(ang), radY + 70*sin(ang)), green, 2);

        // --- CENTER HUD & MISSILE LOGIC ---
        Point screenCenter(display.cols/2, display.rows/2);
        line(display, Point(screenCenter.x-20, screenCenter.y), Point(screenCenter.x+20, screenCenter.y), cyan, 1);
        line(display, Point(screenCenter.x, screenCenter.y-20), Point(screenCenter.x, screenCenter.y+20), cyan, 1);

        // 1. منطق الصاروخ (Movement Logic)
        if (missileActive && locked) {
            // حساب المسافة والاتجاه للهدف
            Point diff = center - missilePos;
            double dist = norm(diff);

            // سرعة الصاروخ (كل ما الرقم زاد كل ما بقى أسرع)
            missilePos += diff * 0.15;

            // رسم الصاروخ (كرة صفراء) وذيل دخان
            line(display, Point(display.cols/2, display.rows), missilePos, Scalar(100,100,100), 1); // ذيل دخان
            circle(display, missilePos, 5, yellow, FILLED);
            putText(display, ">> MISSILE AWAY >>", Point(display.cols/2 - 80, display.rows/2 + 150), FONT_HERSHEY_SIMPLEX, 0.6, red, 2);

            // لو قربنا من الهدف (Impact)
            if (dist < 20) {
                missileActive = false;
                explosionActive = true;
                explosionTimer = 0;
                logData.push_back("TARGET HIT!!");
            }
        }

        // 2. منطق الانفجار (Explosion Logic)
        if (explosionActive) {
            drawExplosion(display, center, explosionTimer);
            putText(display, "!! IMPACT CONFIRMED !!", Point(center.x - 100, center.y - 50), FONT_HERSHEY_SIMPLEX, 0.7, red, 2);
            explosionTimer++;
            if (explosionTimer > 10) explosionActive = false; // الانفجار بيختفي بعد شوية
        }

        if (locked) {
            if (!explosionActive) { // عشان ميغطيش على الانفجار
                drawBracket(display, targetBox.x-10, targetBox.y-10, targetBox.width+20, targetBox.height+20, red);
                circle(display, center, 5, red, FILLED);
                putText(display, "LOCK", Point(targetBox.x, targetBox.y-20), FONT_HERSHEY_SIMPLEX, 0.8, red, 2);
            }
        } else {
            // لو اللوك ضاع والطاروخ في الجو، الصاروخ يضيع
            if (missileActive) { missileActive = false; logData.push_back("MISSILE LOST"); }

            drawBracket(display, screenCenter.x-100, screenCenter.y-100, 200, 200, white);
            if(counter % 40 < 20) putText(display, "NO TARGET", Point(screenCenter.x-60, screenCenter.y+130), FONT_HERSHEY_SIMPLEX, 0.7, red, 1);
        }

        imshow("EGY_ADS_V2", display);
        counter++;

        // --- INPUT HANDLING ---
        int key = waitKey(30);
        if (key == 27) break; // ESC to exit

        // زرار المسافة (SPACE) لإطلاق الصاروخ
        if (key == 32 && locked && !missileActive && !explosionActive) {
            missileActive = true;
            // الصاروخ بينطلق من تحت في النص
            missilePos = Point(display.cols/2, display.rows);
            logData.push_back("LAUNCH DETECTED!");
        }
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
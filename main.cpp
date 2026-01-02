#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main() {
    // 1. فتح كاميرا اللابتوب (0 هو رقم الكاميرا الافتراضية)
    VideoCapture cap(0);

    if (!cap.isOpened()) {
        cout << "Error: Could not open webcam." << endl;
        return -1;
    }

    // تعريف مجال اللون اللي هنتتبعه (هنا ظابطينه على الأزرق)
    // لو عايز لون تاني غير القيم دي (Low - High)
    // Blue Color Range in HSV
    Scalar lowerColor(100, 150, 0); 
    Scalar upperColor(140, 255, 255);

    Mat frame, hsv, mask;

    while (true) {
        // قراءة الفريم من الكاميرا
        cap >> frame;
        if (frame.empty()) break;

        // قلب الصورة عشان تكون زي المرايا (اختياري)
        flip(frame, frame, 1);

        // 2. تحويل الألوان من BGR إلى HSV
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // 3. عزل اللون (Thresholding)
        inRange(hsv, lowerColor, upperColor, mask);

        // تنظيف الشوشرة (Noise Reduction) باستخدام Morphological Ops
        erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        // 4. إيجاد حدود الجسم (Contours)
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // لو لقينا جسم بنفس اللون
        if (!contours.empty()) {
            // نجيب أكبر جسم بس (عشان نتجاهل النقط الصغيرة الغلط)
            double maxArea = 0;
            int maxIdx = -1;
            for (int i = 0; i < contours.size(); i++) {
                double area = contourArea(contours[i]);
                if (area > maxArea) {
                    maxArea = area;
                    maxIdx = i;
                }
            }

            if (maxIdx != -1 && maxArea > 500) { // شرط المساحة عشان ميرسمش على حاجات صغيرة
                // رسم مربع التتبع
                Rect box = boundingRect(contours[maxIdx]);
                
                // ستايل "عسكري" أخضر
                Scalar hudColor(0, 255, 0); 
                
                // رسم المربع
                rectangle(frame, box, hudColor, 2);

                // رسم علامة + في نص المربع (Crosshair)
                int cx = box.x + box.width / 2;
                int cy = box.y + box.height / 2;
                line(frame, Point(cx - 10, cy), Point(cx + 10, cy), hudColor, 2);
                line(frame, Point(cx, cy - 10), Point(cx, cy + 10), hudColor, 2);

                // كتابة البيانات (HUD Text)
                string coords = "X:" + to_string(cx) + " Y:" + to_string(cy);
                putText(frame, "TARGET LOCKED [ACTIVE]", Point(box.x, box.y - 25), FONT_HERSHEY_SIMPLEX, 0.6, hudColor, 2);
                putText(frame, coords, Point(box.x, box.y - 10), FONT_HERSHEY_PLAIN, 1, hudColor, 1);
            }
        } else {
            // لو مفيش هدف
            putText(frame, "SCANNING...", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
        }

        // عرض النتيجة
        imshow("Defense Tech Tracker - C++", frame);
        
        //  عشان تشوف الكمبيوتر شايف إيه 
        // imshow("Computer Vision Mask", mask);

        // الخروج بزرار ESC
        if (waitKey(30) == 27) break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
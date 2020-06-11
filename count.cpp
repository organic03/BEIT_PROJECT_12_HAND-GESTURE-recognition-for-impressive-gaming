#include<iostream>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<opencv2\core\core.hpp>
#include<opencv2\video\background_segm.hpp>
#include<Windows.h>
using namespace cv;
using namespace std;

//functions prototypes
void on_trackbar(int, void*);
void createTrackbars();
void showimgcontours(Mat &threshedimg, Mat &original);
void toggle(int key);
void morphit(Mat &img);
void blurthresh(Mat &img);
void condefects(vector<Vec4i> convexityDefectsSet, vector<Point> mycontour, Mat &frame);

//function prototypes ends here

//boolean toggles

bool domorph = false;
bool doblurthresh = false;
bool showchangedframe = false;
bool showcontours = false;
bool showhull = false;
bool showcondefects = false;

//boolean toggles end


int H_MIN = 0;
int H_MAX = 255;
int S_MIN = 0;
int S_MAX = 255;
int V_MIN = 0;
int V_MAX = 255;

int kerode = 1;
int kdilate = 1;
int kblur = 1;
int threshval = 0;


int main(void)
{
	createTrackbars();
	on_trackbar(0, 0);

	Mat frame, hsvframe, rangeframe;
	int key;
	VideoCapture cap(0);
	while ((key = waitKey(30)) != 27)
	{
		toggle(key);
		cap >> frame;
		flip(frame, frame, 180);
		cvtColor(frame, hsvframe, COLOR_BGR2HSV);

		inRange(hsvframe, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), rangeframe);

		if (domorph)
			morphit(rangeframe);

		if (doblurthresh)
			blurthresh(rangeframe);

		if (showcontours)
			showimgcontours(rangeframe, frame);

		if (showchangedframe)
			imshow("Camera", frame);
		else
			imshow("Camera", rangeframe);

	}

}


void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed
	if (kerode == 0)
		kerode = 1;
	if (kdilate == 0)
		kdilate = 1;
	if (kblur == 0)
		kblur = 1;
}
void createTrackbars()
{
	String trackbarWindowName = "TrackBars";
	namedWindow(trackbarWindowName, WINDOW_NORMAL);
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);
	createTrackbar("Erode", trackbarWindowName, &kerode, 31, on_trackbar);
	createTrackbar("Dilate", trackbarWindowName, &kdilate, 31, on_trackbar);
	createTrackbar("Blur", trackbarWindowName, &kblur, 255, on_trackbar);
	createTrackbar("Thresh", trackbarWindowName, &threshval, 255, on_trackbar);

}

void morphit(Mat &img)
{
	erode(img, img, getStructuringElement(MORPH_RECT, Size(kerode, kerode)));
	dilate(img, img, getStructuringElement(MORPH_RECT, Size(kdilate, kdilate)));
}
void blurthresh(Mat &img)
{
	//medianBlur(img,img,kblur%2+3+kblur);
	blur(img, img, Size(kblur, kblur), Point(-1, -1), BORDER_DEFAULT);
	threshold(img, img, threshval, 255, THRESH_BINARY_INV);
}
void toggle(int key)
{

	//toggle line start
	if (key == 'm')
		domorph = !domorph;
	if (key == 'b')
		doblurthresh = !doblurthresh;
	if (key == 'r')
		showchangedframe = !showchangedframe;
	if (key == 'c')
		showcontours = !showcontours;
	if (key == 'h')
		showhull = !showhull;
	if (key == 'd')
		showcondefects = !showcondefects;
	//toggle line end
}

void showimgcontours(Mat &threshedimg, Mat &original)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	int largest_area = 0;
	int largest_contour_index = 0;

	findContours(threshedimg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	/// Find the convex hull,contours and defects for each contour
	vector<vector<Point> >hull(contours.size());
	vector<vector<int> >inthull(contours.size());
	vector<vector<Vec4i> >defects(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		convexHull(Mat(contours[i]), hull[i], false);
		convexHull(Mat(contours[i]), inthull[i], false);
		if (inthull[i].size()>3)
			convexityDefects(contours[i], inthull[i], defects[i]);
	}
	//find  hulland contour and defects end here
	//this will find largest contour
	for (int i = 0; i< contours.size(); i++) // iterate through each contour. 
	{
		double a = contourArea(contours[i], false);  //  Find the area of contour
		if (a>largest_area)
		{
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
		}

}
	//search for largest contour has end

	if (contours.size() > 0)
	{
		drawContours(original, contours, largest_contour_index, CV_RGB(0, 255, 0), 2, 8, hierarchy);
		//if want to show all contours use below one
		//drawContours(original,contours,-1, CV_RGB(0, 255, 0), 2, 8, hierarchy);
		if (showhull)
			drawContours(original, hull, largest_contour_index, CV_RGB(0, 0, 255), 2, 8, hierarchy);
		//if want to show all hull, use below one
		//drawContours(original,hull,-1, CV_RGB(0, 255, 0), 2, 8, hierarchy);
		if (showcondefects)
			condefects(defects[largest_contour_index], contours[largest_contour_index],original);
	}
}

void condefects(vector<Vec4i> convexityDefectsSet, vector<Point> mycontour, Mat &original)
{
	Point2f mycenter;
	float radii;
	long double fingers=0;
	INPUT ip;
	minEnclosingCircle(mycontour,mycenter,radii);
	circle(original,mycenter,10,CV_RGB(0,0,255),2,8);
	cout << "==start==" << endl;
	for (int cDefIt = 0; cDefIt < convexityDefectsSet.size(); cDefIt++) {

		int startIdx = convexityDefectsSet[cDefIt].val[0]; Point ptStart(mycontour[startIdx]);

		int endIdx = convexityDefectsSet[cDefIt].val[1]; Point ptEnd(mycontour[endIdx]);

		int farIdx = convexityDefectsSet[cDefIt].val[2]; Point ptFar(mycontour[farIdx]);

		double depth = static_cast<double>(convexityDefectsSet[cDefIt].val[3]) / 256;
		//cout << "depth" << depth << endl;
		//display start points of finger
	
		if (depth>11 && ptStart.y<mycenter.y)
		{
			circle(original, ptStart, 4, CV_RGB(255, 0,0), 4);
			fingers++;
			
		}

	}

    if(fingers == 1)
	{
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = VK_LEFT; 
    ip.ki.dwFlags = 0; 
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));

	}
		else if(fingers == 2)
	{
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = VK_UP; 
    ip.ki.dwFlags = 0; 
    SendInput(1, &ip, sizeof(INPUT)); 
    ip.ki.dwFlags = KEYEVENTF_KEYUP; 
    SendInput(1, &ip, sizeof(INPUT));

	}
		else if(fingers == 3)
	{
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = VK_DOWN; 
    ip.ki.dwFlags = 0; 
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP; 
    SendInput(1, &ip, sizeof(INPUT));
	}
		else if(fingers == 4)
	{
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = VK_RIGHT; 
    ip.ki.dwFlags = 0; 
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP; 
    SendInput(1, &ip, sizeof(INPUT));
	}
		else if(fingers == 5)
	{
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = 0x45;
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP; 
    SendInput(1, &ip, sizeof(INPUT));
	}



	//display fingers on image
	putText(original,"Fingers : "+to_string(fingers),Point(50,100),2,2,CV_RGB(0,255,0),4,8);
	cout << "==end line==" << endl;
}// condefects ends here

	

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/background_segm.hpp>
#include <cv.h>

#include <iostream>

#include <cvblob.h>

//#define MARGIN 50
//#define FR_W 320
//#define FR_H 176

void usage() {
	std::cout<<"Usage: ./bsgmmtracker <file> -d <MORPH_ELLIPSE size> -hv 0/1 -m <margin>"<<std::endl;
}

cv::Mat frame, fgMask;//, res;
IplImage *frame_, *fgMask_, *labelImg;
cv::Ptr<cv::BackgroundSubtractor> pMOG;
cvb::CvBlobs blobs;
cvb::CvTracks tracks;
std::map<cvb::CvID, CvPoint2D64f > last_poses;
CvPoint2D64f last_pos;
CvPoint2D64f cur_pos;
int hv;
int line_pos;
int FR_W, FR_H, MARGIN;

int main(int argc, char** argv) {

	if ( argc<8 ) {
		usage();
		return 1;
	}

	int count = 0, countLR = 0, countRL = 0;
	cv::VideoCapture cap(argv[1]); 
	pMOG = new cv::BackgroundSubtractorMOG;
	hv = atoi(argv[5]);

	cap >> frame;
	FR_W = frame.cols;
	FR_H = frame.rows;
	MARGIN = atoi(argv[7]);

	if (!hv)
		line_pos = FR_W - MARGIN;
	else
		line_pos = FR_H - MARGIN;

	while(1) {
		cap.read(frame);
		if (frame.empty())
			break;

		//cv::resize(frame, frame, cv::Size(), 3, 3);
		//process data
		pMOG->operator()(frame, fgMask);
		//cv::erode(fgMask, fgMask, cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(10,10)));//atoi(argv[3]), atoi(argv[3]))));
		cv::dilate(fgMask, fgMask, cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(atoi(argv[3]), atoi(argv[3]))));
		frame_ = new IplImage(frame);
		fgMask_ = new IplImage(fgMask);
		labelImg = cvCreateImage(cvSize(frame.cols, frame.rows), IPL_DEPTH_LABEL, 1);
		unsigned int result = cvb::cvLabel(fgMask_, labelImg, blobs);
		
		cvb::cvRenderBlobs(labelImg, blobs, frame_, frame_, CV_BLOB_RENDER_BOUNDING_BOX|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_ANGLE);//|CV_BLOB_RENDER_TO_STD);
		cvb::cvFilterByArea(blobs, 500, 50000);
		cvb::cvUpdateTracks(blobs,tracks,200.,5);
		cvb::cvRenderTracks(tracks,frame_,frame_, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);//|CV_TRACK_RENDER_TO_STD);//|CV_TRACK_RENDER_BOUNDING_BOX);
		
		for (std::map<cvb::CvID,cvb::CvTrack*>::iterator track_it = tracks.begin(); track_it!=tracks.end(); track_it++) {
			cvb::CvID id = (*track_it).first;
		    cvb::CvTrack* track = (*track_it).second;
		    cur_pos = track->centroid;
		    
		    if (track->inactive == 0) {
		    	//std::cout<<id<<" "<<cur_pos.x<<" "<<cur_pos.y<<std::endl;
		    	if ( last_poses.count(id) ) {
		    		std::map<cvb::CvID, CvPoint2D64f>::iterator pose_it = last_poses.find(id);
		    		last_pos = pose_it->second;
		    		last_poses.erase(pose_it);
		    	}
		    	last_poses.insert(std::pair<cvb::CvID, CvPoint2D64f>(id,cur_pos));
		    	if (!hv) {
		    		if (cur_pos.x>line_pos && last_pos.x<line_pos) {
		    			count++;
		    			countLR++;
		    		}
		    	} else {
		    		if (cur_pos.y>line_pos && last_pos.y<line_pos) {
		    			count++;
		    			countLR++;
		    		}
		    		if (cur_pos.y<line_pos && last_pos.y>line_pos) {
		    			count++;
		    			countRL++;
		    		}
		    	}
		    } else {
		    	if ( last_poses.count(id)) {
		    		last_poses.erase(last_poses.find(id));
		    	}
		    }
		    //int vel_x = cur_pos.x - last_pos.x;
		    //int vel_y = cur_pos.y - last_pos.y;
		    //std::cout << "id: " <<id<<" velX: "<<vel_x<<" velY: "<<vel_y<<"\n";
		    //std::cout<<last_pos.x<<" "<<cur_pos.x<<"\n";
		    //std::cout<<last_poses.size()<<"\n";
		    //for(std::map<cvb::CvID, CvPoint2D64f>::iterator it = last_poses.begin();it!=last_poses.end();it++) {
		    //	std::cout<<"IDs in map: "<<it->first<<" ";
		    //}
		    //std::cout<<"\n";
		}

		//display
		if (!hv) {
			cv::line(frame, cv::Point(line_pos, 0), cvPoint(line_pos, FR_H), cv::Scalar(0,255,0), 2);
			cv::putText(frame, "HORIZONTAL",cv::Point(10,15), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
			cv::putText(frame, "COUNT: "+std::to_string(count), cv::Point(10,30), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
			cv::putText(frame, "LEFT->RIGHT: "+std::to_string(countLR), cv::Point(10,45), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
			cv::putText(frame, "RIGHT->LEFT: "+std::to_string(countRL), cv::Point(10,60), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
		} else {
			cv::line(frame, cv::Point(0, line_pos), cvPoint(FR_W, line_pos), cv::Scalar(0,255,0), 2);
			cv::putText(frame, "VERTICAL",cv::Point(10,15), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
			cv::putText(frame, "COUNT: "+std::to_string(count), cv::Point(10,30), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
			cv::putText(frame, "UP->DOWN: "+std::to_string(countLR), cv::Point(10,45), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
			cv::putText(frame, "DOWN->UP: "+std::to_string(countRL), cv::Point(10,60), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255));
		}	
			//res = new 
			//cv::Mat res(FR_H, 2*FR_W, CV_8UC3);
			//std::cout<<res.size().height<<" "<<res.size().width<<"\n";
			//res.adjustROI(0, 0, 0, -FR_W);
    		//frame.copyTo(res);
    		//res.adjustROI(0, 0, -FR_W, FR_W);
    		//fgMask.copyTo(res);
    		//res.adjustROI(0, 0, FR_W, 0);
    		//cv::imshow("RESULT", res);
			cv::imshow("FRAME", frame); 
			cv::imshow("FGMASK", fgMask);

		if ( cv::waitKey(33)==27 ) break;
	}
	return 0;
}
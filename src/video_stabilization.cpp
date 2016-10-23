#include "opencv_all.hpp"

int main(void)
{
    // Open an video and get the reference image and feature points
    cv::VideoCapture video;
    if (!video.open("images/traffic.avi")) return -1;

    cv::Mat gray_ref;
    video >> gray_ref;
    if (gray_ref.empty())
    {
        video.release();
        return -1;
    }
    if (gray_ref.channels() > 1) cv::cvtColor(gray_ref, gray_ref, CV_RGB2GRAY);

    std::vector<cv::Point2f> point_ref;
    cv::goodFeaturesToTrack(gray_ref, point_ref, 2000, 0.01, 10);
    if (point_ref.size() < 4)
    {
        video.release();
        return -1;
    }

    // Run and show video stabilization
    while (true)
    {
        // Grab an image from the video
        cv::Mat image, gray;
        video >> image;
        if (image.empty()) break;
        if (image.channels() > 1) cv::cvtColor(image, gray, CV_RGB2GRAY);
        else                      gray = image.clone();

        // Extract optical flow and calculate planar homography
        std::vector<cv::Point2f> point;
        std::vector<uchar> m_status;
        cv::Mat err;
        cv::calcOpticalFlowPyrLK(gray_ref, gray, point_ref, point, m_status, err);
        cv::Mat H = cv::findHomography(point, point_ref, cv::RANSAC);

        // Synthesize a stabilized image
        cv::Mat warp;
        cv::warpPerspective(image, warp, H, cv::Size(image.cols, image.rows));

        // Show the original and rectified images together
        for (size_t i = 0; i < point_ref.size(); i++)
            cv::line(image, point_ref[i], point[i], cv::Scalar(0, 0, 255));
        cv::hconcat(image, warp, image);
        cv::imshow("3DVT Tutorial: Video Stabilization", image);
        if (cv::waitKey(1) == 27) break; // "ESC" key
    }

    video.release();
    return 0;
}

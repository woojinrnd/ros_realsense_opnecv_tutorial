// #include <ros/ros.h>
// #include <cv_bridge/cv_bridge.h>
// // #include <opencv2/highgui/highgui.hpp>
// #include <opencv4/opencv2/highgui/highgui.hpp>
// #include <librealsense2/rs.hpp>

// using namespace cv;
// using namespace std;

// int main(int argc, char **argv)
// {
//   ros::init(argc, argv, "ros_realsense_opencv_tutorial");
//   ros::NodeHandle nh;

//   cout << "OpenCV version : " << CV_VERSION << endl;
//   cout << "Major version : "  << CV_MAJOR_VERSION << endl;

//   rs2::pipeline pipe;
//   rs2::config cfg;
//   rs2::frameset frames;
//   rs2::frame color_frame;

//   cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);

//   //kwj
//   cfg.enable_stream(RS2_STREAM_INFRARED, 640, 480, RS2_FORMAT_Y8, 30);
//   cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);

//   pipe.start(cfg);

//   for(int i=0; i < 30; i ++)
//   {
//     frames = pipe.wait_for_frames();
//   }

//   namedWindow("RGB", WINDOW_AUTOSIZE);
//   namedWindow("INFRARED", WINDOW_AUTOSIZE);
//   namedWindow("DEPTH", WINDOW_AUTOSIZE);



//   ros::Rate loop_rate(30);

//   while(ros::ok())
//   {
//     frames = pipe.wait_for_frames();
//     rs2::frame ir_frame = frames.first(RS2_STREAM_INFRARED); //kwj
//     rs2::frame depth_frame = frames.get_depth_frame(); //kwj
//     color_frame = frames.get_color_frame();
//     Mat color(Size(640,480), CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
//     Mat ir(Size(640, 480), CV_8UC1, (void *)ir_frame.get_data(), Mat::AUTO_STEP); //kwj
//     Mat depth(Size(640, 480), CV_8UC1, (void *)depth_frame.get_data(), Mat::AUTO_STEP); //kwj
    
//     equalizeHist( ir, ir ); //kwj
//     equalizeHist( depth, depth ); //kwj
//     applyColorMap(ir, ir, COLORMAP_JET); //kwj
//     applyColorMap(depth, depth, COLORMAP_JET); //kwj

    
//     imshow("RGB", color);
//     imshow("INFRARED", ir); //KWJ
//     imshow("DEPTH", depth); //KWJ


//     if(waitKey(10)==27) break;
//     loop_rate.sleep();
//     ros::spinOnce();
//   }
//   return 0;
// }








// #include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
// #include <opencv2/opencv.hpp>   // Include OpenCV API

// using namespace cv;

// bool is_yaw{false};
// bool is_roll{false};

// cv::RotateFlags angle(cv::RotateFlags::ROTATE_90_CLOCKWISE);

// void changeMode() {
//     static int i = 0;

//     i++;
//     if (i==5) i=0;

//     switch (i) {
//     case 0:
//         is_yaw = false;
//         is_roll = false;
//         break;
//     case 1:
//         is_yaw = true;
//         is_roll = false;
//         break;
//     case 2:
//         is_yaw = false;
//         is_roll = true;

//         angle = cv::RotateFlags::ROTATE_90_CLOCKWISE;

//         break;
//     case 3:
//         is_yaw = false;
//         is_roll = true;

//         angle = cv::RotateFlags::ROTATE_180;

//         break;
//     case 4:
//         is_yaw = false;
//         is_roll = true;

//         angle = cv::RotateFlags::ROTATE_90_COUNTERCLOCKWISE;

//         break;
//     default:
//         break;
//     }
// }

// int main(int argc, char * argv[]) try
// {
//     // Declare depth colorizer for pretty visualization of depth data
//     rs2::colorizer color_map;

//     // Aligning frames to color size
//     rs2::align depthToColorAlignment(RS2_STREAM_COLOR);

//     // Declare threshold filter for work with dots in range
//     rs2::threshold_filter threshold;
//     float threshold_min = 0.3f;
//     float threshold_max = 1.5f;

//     // Keep dots on the depth frame in range
//     threshold.set_option(RS2_OPTION_MIN_DISTANCE, threshold_min);
//     threshold.set_option(RS2_OPTION_MAX_DISTANCE, threshold_max);

//     // Declare RealSense pipeline, encapsulating the actual device and sensors
//     rs2::pipeline pipe;
//     // Start streaming with default recommended configuration
//     pipe.start();

//     rs2::processing_block procBlock( [&](rs2::frame f, rs2::frame_source& src )
//     {
//         const int origWidth = f.as<rs2::video_frame>().get_width();
//         const int origHeight = f.as<rs2::video_frame>().get_height();

//         cv::Mat image(cv::Size(origWidth, origHeight), CV_16UC1, (void*)f.get_data(), cv::Mat::AUTO_STEP);
//         cv::Mat rotated;

//         if ( !is_yaw && !is_roll )
//             rotated = image;

//         if ( is_yaw ) {
//             int rotWidth(static_cast<int>(threshold_max * 1000));

//             rotated = cv::Mat::zeros(cv::Size(rotWidth, image.size().height), CV_16UC1 );

//             for(int h = 0; h < rotated.size().height; h++) {
//                 for(int w = 0; w < rotated.size().width; w++) {

//                     if ( ( h >= image.size().height ) || ( w >= image.size().width ) )
//                         continue;

//                     unsigned short value = image.at<unsigned short>(h, w);

//                     if ( value == 0 )
//                         continue;

//                     rotated.at<unsigned short>( h, value ) = w;
//                 }
//             }
//         }

//         if (is_roll) {
//             cv::rotate( image, rotated, angle );
//         }

//         auto res = src.allocate_video_frame(f.get_profile(), f, 0, rotated.size().width, rotated.size().height);
//         memcpy((void*)res.get_data(), rotated.data, rotated.size().width * rotated.size().height * 2);

//         src.frame_ready(res);
//     });

//     rs2::frame_queue frame_queue;
//     procBlock.start(frame_queue);

//     while ( true )
//     {
//         // get set of frames
//         rs2::frameset frames = pipe.wait_for_frames(); // Wait for next set of frames from the camera

//         // align images
//         frames = depthToColorAlignment.process(frames);

//         // get depth frames
//         rs2::frame depthFrame = frames.get_depth_frame();

//         // keep points in range from threshold_min to threshold_max
//         depthFrame = threshold.process(depthFrame);

//         // call processing block for handle cloud point
//         procBlock.invoke( depthFrame );
//         depthFrame = frame_queue.wait_for_frame();

//         // Query frame size (width and height)
//         const int w = depthFrame.as<rs2::video_frame>().get_width();
//         const int h = depthFrame.as<rs2::video_frame>().get_height();

//         // Create OpenCV matrix of size (w,h) from the colorized depth data
//         cv::Mat image(cv::Size(w, h), CV_8UC3, (void*)depthFrame.apply_filter(color_map).get_data());

//         // Rescale image for convenience
//         if ( ( image.size().width > 1000 ) || (image.size().height > 1000) )
//             resize( image, image, Size(), 0.5, 0.5);

//         // Update the window with new data
//         imshow("window_name", image);
//         int k = waitKey(1);

//         if ( k == 'q' )
//             break;

//         if ( k == 'c' )
//             changeMode();
//     }

//     return EXIT_SUCCESS;
// }
// catch (const rs2::error & e)
// {
//     std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
//     return EXIT_FAILURE;
// }
// catch (const std::exception& e)
// {
//     std::cerr << e.what() << std::endl;
//     return EXIT_FAILURE;
// }


// This example is derived from the ssd_mobilenet_object_detection opencv demo
// and adapted to be used with Intel RealSense Cameras
// Please see https://github.com/opencv/opencv/blob/master/LICENSE


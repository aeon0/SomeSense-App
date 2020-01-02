
//
//  base64 encoding and decoding with C++.
//  Version: 1.01.00
//

#ifndef BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A
#define BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A

#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);
std::string convert_mat_to_base64_jpg(cv::Mat& img);

#endif /* BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A */
